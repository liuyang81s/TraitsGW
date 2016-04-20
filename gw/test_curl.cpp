#include <iostream>
#include <json-c/json.h>
#include "http_download_domain.h"

using namespace std;

int main()
{
        json_object* gw_object = json_object_new_object();
        json_object_object_add(gw_object, "token", json_object_new_string("fgdfgdfgfdgdfgdfgretre"));

	string strPost(json_object_to_json_string(gw_object));
       	cout << "gw_object.to_string=" << json_object_to_json_string(gw_object) << endl;
	cout << strPost << endl;
        
	string  strUrl = "http://traits.imwork.net:10498/AnalyzeServer/system/test.do";
        string  strResponse;
        bool i = 0;
        HttpDownloadDomain hdd(&i);
        hdd.Post(strUrl, strPost, strResponse);	
	cout << "http post sent" << endl;

	if(strResponse.empty()){
		cout << "response empty" << endl;
		return 1;
	} else
		cout << "strResponse=" << strResponse << endl;

	json_object* new_obj;
	new_obj = json_tokener_parse(strResponse.c_str());
	cout << "new_obj.to_string()=" << json_object_to_json_string(new_obj) << endl;

        json_object_put(new_obj);
	json_object_put(gw_object);

	cout << "http post finished" << endl;

	return 0;
}


