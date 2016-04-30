#include <iostream>
#include <string>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <json-c/json.h>

#include "main.h"
#include "http.h"
#include "dev.h"
#include "gw.h"

#define PACKET_SIZE 256

using namespace std;

bool GW_RUNNING = false;

TraitsGW::TraitsGW()
{
#ifdef TRAITS_DEBUG
	gage_name = "wenduji";	
	factory = "sanfeng";
	gage_type = "1";
	gage_no = "abcde";
	gage_id = "123456";
	
	server_url = "http://traits.imwork.net:10498/AnalyzeServer/system/";
	mode = LISTEN;
#endif
}

TraitsGW::TraitsGW(string url)
{
#ifdef TRAITS_DEBUG
	gage_name = "wenduji";	
	factory = "sanfeng";
	gage_type = "1";
	gage_no = "abcde";
	gage_id = "123456";
	
	mode = LISTEN;
#endif
	server_url = url;
}

TraitsGW::~TraitsGW()
{

}


bool TraitsGW::init()
{
	static string init_url = "init.do";

	json_object* init_object;
    init_object = json_object_new_object();
    json_object_object_add(init_object, "gageName", json_object_new_string(gage_name.c_str()));
    json_object_object_add(init_object, "factory", json_object_new_string(factory.c_str()));
    json_object_object_add(init_object, "gageType", json_object_new_string(gage_type.c_str()));
    json_object_object_add(init_object, "gageNo", json_object_new_string(gage_no.c_str()));
    json_object_object_add(init_object, "id", json_object_new_string(gage_id.c_str()));

    string strPost(json_object_to_json_string(init_object));
#ifdef TRAITS_DEBUG
    cout << strPost << endl;
#endif

    string  strUrl = server_url + init_url;
    string  strResponse;
        bool i = 0;
        HttpDownloadDomain hdd(&i);
        hdd.Post(strUrl, strPost, strResponse);

    json_object_put(init_object);
    
	if(strResponse.empty()){
        cout << "response empty" << endl;
        return false;
    } else {
        cout << "strResponse=" << strResponse << endl;
		return true;
	}
}

bool TraitsGW::heartbeat()
{
	static string hb_url = "refresh.do";

	json_object* hb_object;
    hb_object = json_object_new_object();
    json_object_object_add(hb_object, "id", json_object_new_string(gage_id.c_str()));

    string strPost(json_object_to_json_string(hb_object));
#ifdef TRAITS_DEBUG    
	cout << strPost << endl;
#endif

    string  strUrl = server_url + hb_url;
        string  strResponse;
        bool i = 0;
        HttpDownloadDomain hdd(&i);
        hdd.Post(strUrl, strPost, strResponse);

    json_object_put(hb_object);

    if(strResponse.empty()){
        cout << "response empty" << endl;
        return false;
    } else {
        cout << "strResponse=" << strResponse << endl;
		//todo:parse strResponse, and handle it
		return true;
	}

}

static void hex2str(uint8_t* str, uint8_t* hex, int size)
{
	for(int i = 0; i < size; i++) {
		sprintf((char*)str + i * 3, "%x ", hex[i]);
	}	
} 


bool TraitsGW::report(uint8_t *packet, int size)
{
    static string url = server_url + "data.do";

	static uint8_t packet_str[PACKET_SIZE * 3 + 1];
	memset(packet_str, 0, PACKET_SIZE * 3 + 1);
	hex2str(packet_str, packet, size);

	static time_t cur_t;
	time(&cur_t);

	json_object* data_object;
    data_object = json_object_new_object();
    json_object_object_add(data_object, "id", json_object_new_string(gage_id.c_str()));
    json_object_object_add(data_object, "data", json_object_new_string((const char *)packet_str));
    json_object_object_add(data_object, "time", json_object_new_string(ctime(&cur_t)));
    json_object_object_add(data_object, "isAnalysis", json_object_new_string("1"));

	
    string strPost(json_object_to_json_string(data_object));
#ifdef TRAITS_DEBUG
    cout << strPost << endl;
#endif

    string  strResponse;
    bool i = 0;
    HttpDownloadDomain hdd(&i);
    hdd.Post(url, strPost, strResponse);

    json_object_put(data_object);
    
	if(strResponse.empty()) {
#ifdef TARAITS_DEBUG
		cout << "response empty" << endl;
#endif
        return false;
    } else {
        cout << "strResponse=" << strResponse << endl;
		return true;
	}
}


void* gw_run(void* arg)
{
	cout << "gw thread running" << endl;		

	TraitsGW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");	

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
		if(false == gw.report(packet_buf, packet_len))
			//todo: retransmit this packet, or log it
			;
	    	
	}

	delete dev;

out:
	cout << "gw thread exit" << endl;
	//todo: log
	
	return 0;
}

