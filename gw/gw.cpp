#include <iostream>
#include <string>
#include <pthread.h>
#include <string.h>
#include <json-c/json.h>

#include "main.h"
#include "http.h"
#include "dev.h"
#include "gw.h"

#define PACKET_SIZE 256

using namespace std;

bool GW_RUNNING = false;

TRAITS_GW::TRAITS_GW()
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

TRAITS_GW::TRAITS_GW(string url)
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

TRAITS_GW::~TRAITS_GW()
{

}


bool TRAITS_GW::init()
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

bool TRAITS_GW::hb()
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
	return true;
	}

}

bool TRAITS_GW::data()
{
	static string data_url = "data.do";

	json_object* hb_object;
    hb_object = json_object_new_object();
    json_object_object_add(hb_object, "id", json_object_new_string(gage_id.c_str()));
    json_object_object_add(hb_object, "data", json_object_new_string("09 33 0A 20"));
    json_object_object_add(hb_object, "time", json_object_new_string("2015-09-17T11:07:47.926+08:00"));
    json_object_object_add(hb_object, "isAnalysis", json_object_new_string("1"));

	
    string strPost(json_object_to_json_string(hb_object));
#ifdef TRAITS_DEBUG
    cout << strPost << endl;
#endif

    string  strUrl = server_url + data_url;
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
		return true;
	}
}

void* gw_run(void* arg)
{
	cout << "gw thread running" << endl;		

	TRAITS_GW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");	

	Device* dev = new TestDevice();
	if(NULL == dev) {
		//todo: log
		cout << "mem allocation failed" << endl;
		goto out;
	}

	while(GW_RUNNING) {
		static uint8_t packet_buf[PACKET_SIZE];
		int packet_len = 0;
		
		pthread_mutex_lock(&rb_mutex);
		while(true) {
			if(rbuffer->size() > 0) {
				packet_len = dev->recognize_packet(rbuffer->get_data()); 				
				if(packet_len > 0) {
					memset(packet_buf, 0, packet_len);
					rbuffer->get(packet_buf, packet_len);
				}
				break;
			}
			pthread_cond_wait(&rb_cond, &rb_mutex);
		}
		pthread_mutex_unlock(&rb_mutex);
		
		//todo: packet to json, send to server
		cout << "packet to json, send to server" << endl;
	}

	delete dev;

out:
	cout << "gw thread exit" << endl;
	//todo: log
	
	return 0;
}

