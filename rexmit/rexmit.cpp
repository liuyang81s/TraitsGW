#include <iostream>
#include <unistd.h>
#include <string>

#include "traits.h"
#include "httptool.h"
#include "packetfilemgr.h"

using namespace std;

int main()
{
	TRAITScode ret = TRAITSE_LAST;
	
	PacketFileMgr pfmgr;
    HttpTool htool;
	
	string filename;
	string data_url(SERVER_URL);
	data_url.append(DATA_URL);
	
	ret = pfmgr.set_dir(FILEBUF_PATH);		
	if(TRAITSE_OK != ret) {
		;//todo:log, led indication
		return 0;		
	}

	while(true) {
		TRAITS_PF_TIME_TYPE pft_type = TRAITS_PF_INVALID;
		ret = pfmgr.get_file(filename, &pft_type);
		if(TRAITSE_OK != ret) {
			cout << "get past file failed" << endl;
			break;//todo: log, led indication
		}

		if(TRAITS_PF_NONE == pft_type) {
			sleep(5);
			cout << "there is no packet file to process" << endl;
		} else if(TRAITS_PF_PAST == pft_type) {
			while(true) {
				string strPost;
				ret = pfmgr.get_record(strPost);
#if 1
	cout << "strPost=" << strPost << endl;
#endif
				if(TRAITSE_ALL_RECORD_SENT == ret) {
					pfmgr.delete_file(filename);
					break;
				} else if (TRAITSE_OK == ret) {
					if(strPost.empty())
						break;

					string  strResponse;
					while(true) { //持续发送，直到成功
					    htool.Post(data_url, strPost, strResponse);
    					if(strResponse.empty()){
#if 1
				        	cout << "response empty" << endl;
#endif
					        sleep(2);
				    	} else {
#if 1
							cout << "stresponse ok" <<endl;
#endif
							pfmgr.update_record();
							break;
						}
					}	
				} else { //get_record() failed
					//todo:log 
					cout << "read past file failed" <<endl;
					break;
				}
			}//while
		} else if(TRAITS_PF_TODAY == pft_type){ //today file
			cout << "today file processing" << endl;
			string strPost;
			ret = pfmgr.get_today_record(strPost);
			if(TRAITSE_OK == ret) {
				if(strPost.empty()) {//读取正常而返回空说明到文件尾
					sleep(5);		 //等待一段时间后再次检查是否有未发送的报文
					continue;
				}
				
				string  strResponse;
				while(true) { //持续发送，直到成功
					htool.Post(data_url, strPost, strResponse);
    				if(strResponse.empty()){
#if 1
			    	cout << "response empty" << endl;
#endif
			       		sleep(2);
				   	} else {
#if 1
						cout << "stresponse ok" <<endl;
#endif
						pfmgr.update_record_today();
						break;
					}
				}//while	
			} else { //get_record() failed
				//todo: log
				cout << "read today file failed" << endl;
				break;
			}
		}	
	}//while

	cout << "rexmit over" << endl;

	return 0;
}


