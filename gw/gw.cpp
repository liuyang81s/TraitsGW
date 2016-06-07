#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <json-c/json.h>
#include <json-c/bits.h>

#include "main.h"
#include "defines.h"
#include "httptool.h"
#include "devs.h"
#include "led.h"
#include "serial.h"
#include "packetfilemgr.h"
#include "traits_elog.h"
#include "gw.h"

#define	CONFIG_PATH "/etc/config/device" 
#define PACKET_SIZE 256
#define HB_PERIOD	(5 * 60)	//heartbeat period 5min

using namespace std;

static string get_attr_from_line(const string& line);

bool GW_RUNNING = false;
bool HB_RUNNING = false;

TraitsGW::TraitsGW()
{
	server_url = "http://traits.imwork.net:10498/AnalyzeServer/system/";
}

TraitsGW::TraitsGW(const string& url)
{
	server_url = url;
}

TRAITScode TraitsGW::init()
{
	led_ok();

	gage_name.clear();
	gage_type.clear();
	gage_no.clear();
	vendor.clear();
	self_id.clear();
	port.clear();
    
    uart_mode = UART_INVALID;
    send_type = SEND_INVALID;
    proto = PROTO_INVALID;
    plan_mode = PLAN_INVALID;
    collect_cycle = 0;

#ifdef TRAITS_DEBUG_SRV
	gage_name = "wenduji";	
	vendor = "sanfeng";
	gage_type = "1";
	gage_no = "abcde";
	self_id = "123456";
	
	uart_mode = UART_POLL;
    proto = PROTO_MODBUS;
    send_type = SEND_HEX;
    plan_mode = PLAN_NONE;
#endif

	//read config file, get port info 
	ifstream config_file(CONFIG_PATH);
	if(!config_file) {
	    log_e("%s not found", CONFIG_PATH);
		return TRAITSE_CONFIG_FILE_NOT_FOUND;	
	}

	string line;
    while(!config_file.eof()){
		getline(config_file, line);
		if(string::npos != line.find("port")) {
			port = get_attr_from_line(line);
			break;
		}
	}  			
#ifdef TRAITS_DEBUG_GW
	cout << "port = " << port << endl;	
#endif
	if(port.empty())	
		return TRAITSE_CONFIG_PARAM_NOT_FOUND;

	tmlist = new TimerList();
	if(NULL == tmlist) {
		log_e("TimerList alloction failed");
        return TRAITSE_MEM_ALLOC_FAILED;
	}

	TRAITScode ret = TRAITSE_LAST;
	ret = tmlist->init();	
	if(TRAITSE_OK != ret) {
		goto failed;
	}	
   
	ret = pfmgr.set_dir(FILEBUF_PATH);    
    if(TRAITSE_OK != ret) {
		goto failed;
    }

	return TRAITSE_OK; 

failed:
	delete tmlist;

	return ret;
}

TraitsGW::~TraitsGW()
{
#if 1
	cout << "~TraitsGW" <<endl;
#endif
    if(NULL != tmlist)
        delete tmlist;
}

static string get_attr_from_line(const string& line)
{
	int i = line.find_first_of('\'');	
	int j = line.find_last_of('\'');

	return line.substr(i + 1, j - i - 1);	
}


string TraitsGW::get_self_id()
{
#define ETH0_MAC_ADDR "/sys/class/net/eth0/address"

	ifstream mac_in(ETH0_MAC_ADDR);
	if(!mac_in) {
		log_e("%s not found, cannot get self id", ETH0_MAC_ADDR);
		return "";
	}	

	string mac;	
	getline(mac_in, mac); 

	string id;
	id.push_back(mac[0]);
	id.push_back(mac[1]);

	id.push_back(mac[3]);
	id.push_back(mac[4]);

	id.push_back(mac[6]);
	id.push_back(mac[7]);

	id.push_back(mac[9]);
	id.push_back(mac[10]);

	id.push_back(mac[12]);
	id.push_back(mac[13]);

	id.push_back(mac[15]);
	id.push_back(mac[16]);

	return id;
}

TRAITScode TraitsGW::request_init()
{
	led_ok();

#ifndef TRAITS_DEBUG_SRV
	//read config file, get gage info 
	ifstream config_file(CONFIG_PATH);
	if(!config_file) {
	    log_e("%s not found", CONFIG_PATH);
		return TRAITSE_CONFIG_FILE_NOT_FOUND;	
	}

	string line;
    while(!config_file.eof()){
		getline(config_file, line);
		
		if(string::npos != line.find("vendor"))
			vendor = get_attr_from_line(line);
		else if(string::npos != line.find("name"))
			gage_name = get_attr_from_line(line);
		else if(string::npos != line.find("type"))
			gage_type = get_attr_from_line(line);
		else if(string::npos != line.find("num"))
			gage_no = get_attr_from_line(line);
		else
			continue;
	}  				
#endif    
#ifdef TRAITS_DEBUG_GW
		cout << "vendor = " << vendor << endl;
		cout << "gage_name = " << gage_name << endl;
		cout << "gage_type = " << gage_type << endl;
		cout << "gage_no = " << gage_no << endl;
#endif		

	if(vendor.empty() || gage_name.empty()
		||gage_type.empty() || gage_no.empty())
		return TRAITSE_CONFIG_PARAM_NOT_FOUND;

#ifndef TRAITS_DEBUG_SRV
	self_id = get_self_id();
#endif
#ifdef TRAITS_DEBUG_GW
		cout << "self_id = " << self_id << endl;  
#endif
	if(self_id == "")
		return TRAITSE_MAC_NOT_FOUND;

	//send 'init' request to server
	json_object* init_object;
    init_object = json_object_new_object();
    json_object_object_add(init_object, "gageName", json_object_new_string(gage_name.c_str()));
    json_object_object_add(init_object, "factory", json_object_new_string(vendor.c_str()));
    json_object_object_add(init_object, "gageType", json_object_new_string(gage_type.c_str()));
    json_object_object_add(init_object, "gageNo", json_object_new_string(gage_no.c_str()));
    json_object_object_add(init_object, "id", json_object_new_string(self_id.c_str()));

    string strPost(json_object_to_json_string(init_object));
    log_d("init_TO_server: %s", strPost.c_str());

    json_object_put(init_object);

    string  strUrl = server_url + INIT_URL;
    string  strResponse;
    HttpTool htool;
    htool.Post(strUrl, strPost, strResponse);
    
	if(strResponse.empty()){
        log_w("init response empty");
        return TRAITSE_RESPONSE_NONE;
    } else {
        log_d("initResponse=%s", strResponse.c_str());
		return init_response_handler(strResponse);
	}
}

TRAITScode TraitsGW::heartbeat()
{
	led_ok();

	json_object* hb_object;
    hb_object = json_object_new_object();
    json_object_object_add(hb_object, "id", json_object_new_string(self_id.c_str()));

    string strPost(json_object_to_json_string(hb_object));
#ifdef TRAITS_DEBUG_HB
	log_d("hb_TO_server: %s", strPost.c_str());
#endif

    string  strUrl = server_url + HB_URL;
    string  strResponse;
    HttpTool hdd;
    hdd.Post(strUrl, strPost, strResponse);

    json_object_put(hb_object);

    if(strResponse.empty()){
        log_w("hb response empty");
        return TRAITSE_RESPONSE_NONE;
    } else {
        log_d("hbResponse=%s", strResponse.c_str());
		return hb_response_handler(strResponse);
	}
}

static void hex2str(uint8_t* str, uint8_t* hex, int size)
{
    int i = 0;
	for(i = 0; i < size; i++) {
        if(hex[i] <= 9)
		    sprintf((char*)str + i * 3, "0%x ", hex[i]);
        else
    		sprintf((char*)str + i * 3, "%2x ", hex[i]);
	}
    str[i * 3 - 1] = 0;    
} 


TRAITScode TraitsGW::report(uint8_t *packet, const int size)
{
	led_ok();

    static string url = server_url + DATA_URL;

	static uint8_t packet_str[PACKET_SIZE * 3 + 1];
	memset(packet_str, 0, PACKET_SIZE * 3 + 1);
	hex2str(packet_str, packet, size);

	static time_t cur_t;
	time(&cur_t);
    static char str_time[30];
    memset(str_time, 0, 30);
    strftime(str_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&cur_t));

	json_object* data_object;
    data_object = json_object_new_object();
    json_object_object_add(data_object, "id", json_object_new_string(self_id.c_str()));
    json_object_object_add(data_object, "data", json_object_new_string((const char *)packet_str));
    json_object_object_add(data_object, "time", json_object_new_string(str_time));
    //todo:0 or 1
    json_object_object_add(data_object, "isAnalysis", json_object_new_int(1));

	
    string strPost(json_object_to_json_string(data_object));
    log_d("data_TO_server: %s", strPost.c_str());

    string  strResponse;
    HttpTool hdd;
    hdd.Post(url, strPost, strResponse);

    json_object_put(data_object);
    
	if(strResponse.empty()) {
		log_w("data packet report failed, write to file");
		//当保存在fat32文件系统上时，显式加上换行符
		//否则所有记录会保存为同一行
		TRAITScode ret = pfmgr.put_today_record(strPost + "\n");
		if(TRAITSE_OK != ret)
			return ret;
		else
	        return TRAITSE_RESPONSE_NONE;
    } else {
        log_d("dataResponse=%s", strResponse.c_str());
		return data_response_handler(strResponse);
	}
}

string TraitsGW::get_port() const
{
	return port;
}

UART_MODE TraitsGW::get_uart_mode() const
{
    return uart_mode;
}

SEND_TYPE TraitsGW::get_send_type() const
{
    return send_type;
}

PROTO_TYPE TraitsGW::get_proto_type() const
{
    return proto;
}

PLAN_MODE TraitsGW::get_plan_mode() const
{
    return plan_mode;
}

TimerList* TraitsGW::get_timerlist() const
{
    return tmlist;
}

TRAITScode TraitsGW::init_response_handler(const string& response)
{
    if(response.empty())
       return TRAITSE_RESPONSE_NONE; 
  
    json_object *temp_obj; 
    json_object *full_obj = json_tokener_parse(response.c_str()); 
    if(is_error(full_obj)){
        log_e("init response string is invalid");
        return TRAITSE_RESPONSE_FORMAT_ERROR;
    }

	TRAITScode ret = TRAITSE_OK;
    string send_content;
    int is_listen;
    int stype;
    int isplan;

    //parse 'code'
    json_object_object_get_ex(full_obj, "code", &temp_obj);
    int srv_ret_code = json_object_get_int(temp_obj);
    
    //parse 'errorMessage'
    json_object_object_get_ex(full_obj, "errorMessage", &temp_obj);
    string errmsg(json_object_get_string(temp_obj));
   
    //parse 'time'
    json_object_object_get_ex(full_obj, "time", &temp_obj);
    string server_time(json_object_get_string(temp_obj));
#ifdef TRAITS_DEBUG_GW
    cout << "server_time = " << server_time << endl;
#endif    

    //parse 'modbusType'
    json_object_object_get_ex(full_obj, "modbusType", &temp_obj);
    int p = json_object_get_int(temp_obj);
    if(0 == p)
        proto = PROTO_NONE;
    else if(1 == p)
        proto = PROTO_MODBUS;
    else if(2 == p)
        proto = PROTO_OTHER;
    else {
        proto = PROTO_INVALID;
        log_e("modbusType invalid: %d", p);
		ret = TRAITSE_RESPONSE_CONTENT_ERROR;
        goto release_json_obj;
    }
      
    //parse 'isListen'
    json_object_object_get_ex(full_obj, "isListen", &temp_obj);
    is_listen = json_object_get_int(temp_obj);
    if(0 == is_listen)
        uart_mode = UART_POLL;
    else if(1 == is_listen)
        uart_mode = UART_LISTEN;
    else {
        uart_mode = UART_INVALID;
        log_e("isListen invalid: %d", is_listen);
		ret = TRAITSE_RESPONSE_CONTENT_ERROR;
        goto release_json_obj;
    }

    //parse 'sendType'
    json_object_object_get_ex(full_obj, "sendType", &temp_obj);
    stype = json_object_get_int(temp_obj);
    if(0 == stype)
        send_type = SEND_NOTHING;
    else if(1 == stype)
        send_type = SEND_ASCII;
    else if(2 == stype)
        send_type = SEND_HEX;
    else {
        send_type = SEND_INVALID;
        log_e("send_type invalid: %d", stype);
		ret = TRAITSE_RESPONSE_CONTENT_ERROR;
        goto release_json_obj;
    }

    //parse 'sendContent'
    json_object_object_get_ex(full_obj, "sendContent", &temp_obj);
    send_content = json_object_get_string(temp_obj);
      
    //parse 'isPlan'
    json_object_object_get_ex(full_obj, "isPlan", &temp_obj);
    isplan = json_object_get_int(temp_obj);
    if(0 == isplan)
        plan_mode = PLAN_NONE;
    else if(1 == isplan)
        plan_mode = PLAN_UPDATE;
    else { 
        plan_mode = PLAN_INVALID;
        log_e("isPlan invalid: %d", isplan);
		ret = TRAITSE_RESPONSE_CONTENT_ERROR;
        goto release_json_obj;
    }

    //parse 'collectCycle'
    json_object_object_get_ex(full_obj, "collectCycle", &temp_obj);
    collect_cycle = json_object_get_int(temp_obj) & 0xff;
	if(0x80 & collect_cycle) {
		log_w("collectCycle invalid: %2x", collect_cycle);
		collect_cycle &= 0x7f;
	}
    
#ifndef TRAITS_TEST
    json_object* timer_obj;
	json_object_object_get_ex(full_obj, "planList", &temp_obj);
    for(int i = 0; i < json_object_array_length(temp_obj); i++){
        timer_obj = json_object_array_get_idx(temp_obj, i);
		string tv = json_object_get_string(timer_obj);
#ifdef TRAITS_DEBUG_GW
        log_i("timer = %s", tv.c_str());
#endif
		//make new timer, add to timerlist
		Timer* tm = new WeeklyTimer(collect_cycle);
		if(NULL == tm){
			log_e("Timer alloction failed");
            tmlist->clean_timers();
			ret = TRAITSE_MEM_ALLOC_FAILED;
            goto release_json_obj;
		}
		if(TRAITSE_OK != tm->set_time(tv)) {
			//todo:log it or led indication
			//plan time format error, but we just ignore it
			//and move to next
            log_e("set_time failed");
			continue;
		}
		tm->onTime = serial_onTime;
		tmlist->add_timer(tm);
    }
#else
       Timer* test_tm;
       test_tm = new Timer();
       struct timeval newtv;
       newtv.tv_sec = 5;
       newtv.tv_usec = 0;
       test_tm->set_time(newtv);
       test_tm->set_period(10); 
       test_tm->onTime = serial_onTime;
       tmlist->add_timer(test_tm);
#endif

release_json_obj:    
#ifndef TRAITS_TEST
    json_object_put(timer_obj);
#endif
    json_object_put(temp_obj);
    json_object_put(full_obj);

    if(TRAITSE_OK != ret || 0 != srv_ret_code){
		led_error();
        if(0 != srv_ret_code) {
            log_w("server return code = %d", srv_ret_code);
            ret = TRAITSE_OK; //packet parse correctly, so we return true
        }
        return ret;
    }

    //set system time
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if(NULL == strptime(server_time.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
        log_e("server time format error");
        //todo: 不设置时间，取RTC时间
    } else {  
        struct timeval tv;
        //考虑到server取时间、传输、客户端处理等过程
        //客户端设置的时间比server时间多加1秒
        tv.tv_sec = mktime(&tm) + 1;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
    }

    return TRAITSE_OK;
}


TRAITScode TraitsGW::data_response_handler(const string& response)
{
    if(response.empty())
       return TRAITSE_RESPONSE_NONE; 
  
    json_object *temp_obj; 
    json_object *full_obj = json_tokener_parse(response.c_str()); 
    if(is_error(full_obj)){
        log_e("data response string is invalid");
        return TRAITSE_RESPONSE_FORMAT_ERROR;
    }

    //parse 'code'
    json_object_object_get_ex(full_obj, "code", &temp_obj);
    int srv_ret_code = json_object_get_int(temp_obj);
    
    //parse 'errorMessage'
    json_object_object_get_ex(full_obj, "errorMessage", &temp_obj);
    string errmsg(json_object_get_string(temp_obj));
   
    json_object_put(full_obj);
    json_object_put(temp_obj);
   
    if(0 != srv_ret_code){
        led_error();
        log_w("server return code = %d", srv_ret_code);
    }

	led_report_success();

    return TRAITSE_OK;
}

TRAITScode TraitsGW::hb_response_handler(const string& response)
{
    if(response.empty())
       return TRAITSE_RESPONSE_NONE; 
 
    TRAITScode ret = TRAITSE_OK;

    json_object *temp_obj; 
    json_object *full_obj = json_tokener_parse(response.c_str()); 
    if(is_error(full_obj)){
        log_e("init response string is invalid");
        return TRAITSE_RESPONSE_FORMAT_ERROR;
    }

    //parse 'code'
    json_object_object_get_ex(full_obj, "code", &temp_obj);
    int srv_ret_code = json_object_get_int(temp_obj);
    
    //parse 'errorMessage'
    json_object_object_get_ex(full_obj, "errorMessage", &temp_obj);
    string errmsg(json_object_get_string(temp_obj));
   
    //parse 'time'
    json_object_object_get_ex(full_obj, "time", &temp_obj);
    string server_time(json_object_get_string(temp_obj));
    
    //parse 'isPlan'
    json_object_object_get_ex(full_obj, "isPlan", &temp_obj);
    int isplan = json_object_get_int(temp_obj);
    if(0 == isplan) {
        plan_mode = PLAN_NONE;
        tmlist->clean_timers();
        return TRAITSE_OK;
    } else if(1 == isplan)
        plan_mode = PLAN_UPDATE;
    else if(2 == isplan) {
        plan_mode = PLAN_REMAIN;
        return TRAITSE_OK;
    } else {
        plan_mode = PLAN_INVALID;
        led_error();
        log_e("plan_mode invalid: %d", isplan);
        return TRAITSE_OK;
    }

    //when plan_mode = PLAN_UPDATE

    //parse 'collectCycle'
    json_object_object_get_ex(full_obj, "collectCycle", &temp_obj);
    collect_cycle = json_object_get_int(temp_obj) & 0xff;

    //parse 'planList'
    json_object_object_get_ex(full_obj, "planList", &temp_obj);
    json_object* timer_obj;
    for(int i = 0; i < json_object_array_length(temp_obj); i++){
        timer_obj = json_object_array_get_idx(temp_obj, i);
        string tv = json_object_get_string(timer_obj);
        log_i("new plan time = %s", tv.c_str());
        
		Timer* tm = new WeeklyTimer(collect_cycle);
        if(NULL == tm) {
            log_e("Timer alloction failed");
            tmlist->clean_timers();
            ret = TRAITSE_MEM_ALLOC_FAILED;
            goto release_json_obj;
        }
        if(TRAITSE_OK != tm->set_time(tv)) {
            log_e("set time failed");
            continue;
        }
        tm->onTime = serial_onTime;
        tmlist->add_timer(tm);
    }

release_json_obj:    
    json_object_put(timer_obj);
    json_object_put(full_obj);
    json_object_put(temp_obj);
    
    return ret;
}

static void rbuffer_log(const char* prefix, uint8_t *buf, int size)        
{
    cout << prefix << ": ";                                                                                     
    cout.fill('0');

    for(int i = 0; i < size; i++)
        cout << setw(2) << hex << (uint32_t)buf[i] << ' ';
    cout << endl;
}

//网络数据转发线程
void* gw_run(void* arg)
{
	log_i("gw thread running...");		

	TraitsGW* gw = (TraitsGW*)arg;

	Device* dev = new SONBEST_SD5110B(0x1);
	if(NULL == dev) {
		log_e("dev alloc failed");
		goto out;
	}

	GW_RUNNING = true;
	while(GW_RUNNING) {
		static uint8_t packet_buf[PACKET_SIZE];
		int packet_len = 0;
		
		pthread_mutex_lock(&rb_mutex);
		while(true) {
			if(rbuffer->size() > 0) {
				packet_len = dev->recognize_packet(rbuffer->get_data(), rbuffer->size()); 				

				//找到合法报文时，取出，继续检测
				//否则，阻塞，等待被唤醒
				if(packet_len > 0) {
					memset(packet_buf, 0, packet_len);
					rbuffer->get(packet_buf, packet_len);
#ifdef TRAITS_DEBUG_GW
                    rbuffer_log("rbuffer", packet_buf, packet_len);
#endif
					break;
				}
			}
			pthread_cond_wait(&rb_cond, &rb_mutex);
		}
		pthread_mutex_unlock(&rb_mutex);
		
		gw->report(packet_buf, packet_len);
	}

	delete dev;

out:
	log_i("gw thread exit...");
	
	return 0;
}

//心跳报文线程
void* hb_run(void* arg)
{
	log_i("heartbeat thread running...");		

	TraitsGW* gw = (TraitsGW*)arg;

	HB_RUNNING = true;
	while(HB_RUNNING) {
#ifdef TRAITS_DEBUG_HB
		cout << "heartbeat" << endl;
#endif
		gw->heartbeat();

		sleep(HB_PERIOD);
	}

	log_i("heartbeat thread exit...");
	
	return 0;
}

