#include <iostream>
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
#include "httptool.h"
#include "dev.h"
#include "serial.h"
#include "gw.h"

#define	CONFIG_PATH "/etc/config/device" 
#define PACKET_SIZE 256
#define HB_PERIOD	30	//heartbeat period 30s

using namespace std;

bool GW_RUNNING = false;
bool HB_RUNNING = false;


TraitsGW::TraitsGW()
{
	init();
	server_url = "http://traits.imwork.net:10498/AnalyzeServer/system/";
}

TraitsGW::TraitsGW(const string& url)
{
	init();
	server_url = url;
}

void TraitsGW::init()
{
	gage_name.clear();
	gage_type.clear();
	gage_no.clear();
	vendor.clear();
	self_id.clear();
    
    uart_mode = UART_INVALID;
    send_type = SEND_INVALID;
    proto = PROTO_INVALID;
    plan_mode = PLAN_INVALID;
    collect_cycle = 0;
	tmlist = new TimerList();
	if(tmlist == NULL) {
		cout << "TimerList alloction failed" << endl;
		//todo: log
        //todo: throw exception
	}	
    
#ifdef TRAITS_DEBUG
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
}

TraitsGW::~TraitsGW()
{
    if(NULL != tmlist)
        delete tmlist;
}

static string get_attr_from_line(string line)
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
		cout << ETH0_MAC_ADDR << " not found" << endl;
		//todo: log
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
	static string init_url = "init.do";

#ifndef TRAITS_DEBUG
	//read config file, get gage info 
	ifstream config_file(CONFIG_PATH);
	if(!config_file) {
		cout << CONFIG_PATH << " not found" << endl;
		//todo: log
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

#ifndef TRAITS_DEBUG
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
#ifdef TRAITS_DEBUG_GW
    cout << strPost << endl;
#endif
    json_object_put(init_object);

    string  strUrl = server_url + init_url;
    cout << "strUrl = " << strUrl << endl;
    string  strResponse;
    HttpTool htool;
    htool.Post(strUrl, strPost, strResponse);
    
	if(strResponse.empty()){
#ifdef TRAITS_DEBUG_GW
        cout << "response empty" << endl;
#endif
		//todo:log
        return TRAITSE_RESPONSE_NONE;
    } else {
#ifdef TRAITS_DEBUG_GW
        cout << "strResponse=" << strResponse << endl;
#endif
		return init_response_handler(strResponse);
	}
}

TRAITScode TraitsGW::heartbeat()
{
	static string hb_url = "refresh.do";

	json_object* hb_object;
    hb_object = json_object_new_object();
    json_object_object_add(hb_object, "id", json_object_new_string(self_id.c_str()));

    string strPost(json_object_to_json_string(hb_object));
#ifdef TRAITS_DEBUG_HB
	cout << strPost << endl;
#endif

    string  strUrl = server_url + hb_url;
        string  strResponse;
        HttpTool hdd;
        hdd.Post(strUrl, strPost, strResponse);

    json_object_put(hb_object);

    if(strResponse.empty()){
#ifdef TRAITS_DEBUG_HB
        cout << "response empty" << endl;
#endif
        return TRAITSE_RESPONSE_NONE;
    } else {
#ifdef TRAITS_DEBUG_HB
        cout << "strResponse=" << strResponse << endl;
#endif
		//todo:parse strResponse, and handle it
		return TRAITSE_OK;
	}

}

static void hex2str(uint8_t* str, uint8_t* hex, int size)
{
	for(int i = 0; i < size; i++) {
		sprintf((char*)str + i * 3, "%x ", hex[i]);
	}	
} 


TRAITScode TraitsGW::report(uint8_t *packet, const int size)
{
    static string url = server_url + "data.do";

#if 1    
    cout << "report" << endl;
#endif

	static uint8_t packet_str[PACKET_SIZE * 3 + 1];
	memset(packet_str, 0, PACKET_SIZE * 3 + 1);
	hex2str(packet_str, packet, size);

	static time_t cur_t;
	time(&cur_t);

	json_object* data_object;
    data_object = json_object_new_object();
    json_object_object_add(data_object, "id", json_object_new_string(self_id.c_str()));
    json_object_object_add(data_object, "data", json_object_new_string((const char *)packet_str));
    json_object_object_add(data_object, "time", json_object_new_string(ctime(&cur_t)));
    //todo:0 or 1
    json_object_object_add(data_object, "isAnalysis", json_object_new_int(1));

	
    string strPost(json_object_to_json_string(data_object));
#ifdef TRAITS_DEBUG_GW
    cout << strPost << endl;
#endif

    string  strResponse;
    HttpTool hdd;
    hdd.Post(url, strPost, strResponse);

    json_object_put(data_object);
    
	if(strResponse.empty()) {
#ifdef TARAITS_DEBUG_GW 
        cout << "response empty" << endl;
#endif
        return TRAITSE_RESPONSE_NONE;
    } else {
        cout << "strResponse=" << strResponse << endl;
		return TRAITSE_OK;
	}
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
        cout << "init response string is invalid" << endl;
        //todo: log
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

#ifndef TRAITS_DEBUG    
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
        cout << "'modbusType' invalid" << endl;
        //todo: log
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
        cout << "'isListen' invalid" << endl;
        //todo: log
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
        cout << "'send_type' invalid" << endl;
        //todo: log
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
        plan_mode = PLAN_REMAIN;
    else if(2 == isplan)
        plan_mode = PLAN_UPDATE;
    else { 
        plan_mode = PLAN_INVALID;
        cout << "'isPlan' invalid" << endl;
        //todo: log
		ret = TRAITSE_RESPONSE_CONTENT_ERROR;
        goto release_json_obj;
    }

    //parse 'collectCycle'
    json_object_object_get_ex(full_obj, "collectCycle", &temp_obj);
    collect_cycle = json_object_get_int(temp_obj) & 0xff;
    
	//parse 'planList'
    //tmlist->init() maybe not necessary
	//tmlist->init();
    
	json_object_object_get_ex(full_obj, "planList", &temp_obj);
    json_object* timer_obj;
    for(int i = 0; i < json_object_array_length(temp_obj); i++){
        timer_obj = json_object_array_get_idx(temp_obj, i);
		string tv = json_object_to_json_string(timer_obj);
        cout << "timer = " << tv << endl;
		//make new timer, add to timerlist
		Timer* tm = new WeeklyTimer(collect_cycle);
		if(NULL == tm){
			cout << "Timer alloction failed" << endl;
			//todo:log
            tmlist->clean_timers();
			ret = TRAITSE_MEM_ALLOC_FAILED;
            goto release_json_obj;
		}
		if(false == tm->set_time(tv)) {
			//todo:log it
			//or led indication
			//plan time format error, but we just ignore it
			//and move to next
			continue;
		}
		tm->onTime = serial_onTime;
		tmlist->add_timer(tm);
    }
 
release_json_obj:    
    json_object_put(timer_obj);
    json_object_put(temp_obj);
    json_object_put(full_obj);

    if(TRAITSE_OK != ret || 0 != srv_ret_code){
        //todo: LED error indication
        if(0 != srv_ret_code) {
            //todo: log 'server ret 1'
            ret = TRAITSE_OK; //packet parse correctly, so we return true
        }
        return ret;
    }
#endif    

    //set system time
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if(NULL == strptime(server_time.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
        cout << "server time format error" << endl;
        //todo:log 
        //不设置时间，取RTC时间
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
        cout << "init response string is invalid" << endl;
        //todo: log
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
    
    return TRAITSE_OK;
}


TRAITScode TraitsGW::hb_response_handler(const string& response)
{
    if(response.empty())
       return TRAITSE_RESPONSE_NONE; 
  
    json_object *temp_obj; 
    json_object *full_obj = json_tokener_parse(response.c_str()); 
    if(is_error(full_obj)){
        cout << "init response string is invalid" << endl;
        //todo: log
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
    if(0 == isplan)
        plan_mode = PLAN_NONE;
    else if(1 == isplan)
        plan_mode = PLAN_REMAIN;
    else if(2 == isplan)
        plan_mode = PLAN_UPDATE;
    else 
        plan_mode = PLAN_INVALID;

    //parse 'planList'
    json_object_object_get_ex(full_obj, "planList", &temp_obj);
    json_object* timer_obj;
    for(int i = 0; i < json_object_array_length(temp_obj); i++){
        timer_obj = json_object_array_get_idx(temp_obj, i);
        cout << "timer = " << json_object_to_json_string(timer_obj);
        //todo:add timer list
    }
    json_object_put(timer_obj);

    //parse 'collectCycle'
    json_object_object_get_ex(full_obj, "collectCycle", &temp_obj);
    collect_cycle = json_object_get_int(temp_obj) & 0xff;
    
    json_object_put(full_obj);
    json_object_put(temp_obj);

    
    return TRAITSE_OK;
}

//网络数据转发线程
void* gw_run(void* arg)
{
	cout << "gw thread running" << endl;		

	TraitsGW* gw = (TraitsGW*)arg;

	Device* dev = new TestDevice();
	if(NULL == dev) {
		//todo: log
		cout << "mem allocation failed" << endl;
		goto out;
	}

	GW_RUNNING = true;
	while(GW_RUNNING) {
		static uint8_t packet_buf[PACKET_SIZE];
		int packet_len = 0;
		
		pthread_mutex_lock(&rb_mutex);
		while(true) {
			if(rbuffer->size() > 0) {
				packet_len = dev->recognize_packet(rbuffer->get_data()); 				

				//找到合法报文时，取出，继续检测
				//否则，阻塞，等待被唤醒
				if(packet_len > 0) {
					memset(packet_buf, 0, packet_len);
					rbuffer->get(packet_buf, packet_len);
					break;
				}
			}
			pthread_cond_wait(&rb_cond, &rb_mutex);
		}
		pthread_mutex_unlock(&rb_mutex);
		
		cout << "packet to json, send to server" << endl;
		if(false == gw->report(packet_buf, packet_len))
			//todo: retransmit this packet, or log it
			;
	    	
	}

	delete dev;

out:
	cout << "gw thread exit" << endl;
	//todo: log
	
	return 0;
}

//心跳报文线程
void* hb_run(void* arg)
{
	cout << "hb thread running" << endl;		

	TraitsGW* gw = (TraitsGW*)arg;

	HB_RUNNING = true;
	while(HB_RUNNING) {
#ifdef TRAITS_DEBUG_HB
		cout << "heartbeat" << endl;
#endif
		gw->heartbeat();

		sleep(HB_PERIOD);
	    //todo: handle the response	
	}

	cout << "hb thread exit" << endl;
	
	return 0;
}

