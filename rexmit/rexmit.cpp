#include <iostream>
#include <unistd.h>
#include <string>

#include "traits.h"
#include "httptool.h"
#include "packetfilemgr.h"

using namespace std;

int main()
{
	PacketFileMgr pfmgr;
	string filename;
    HttpTool htool;
	string data_url(SERVER_URL);
#if 0
	data_url.append(DATA_URL);
#else
	data_url.append(INIT_URL);
#endif
	TRAITScode ret = TRAITSE_LAST;
	
	ret = pfmgr.set_dir(FILEBUF_PATH);		
	if(TRAITSE_OK != ret) {
		;//todo:log, led indication
		return 0;		
	}

	while(true) {
		ret = pfmgr.get_past_file(filename);
		if(TRAITSE_OK != ret) {
			cout << "get past file failed" << endl;
			break;//todo: log, led indication
		}
	
		if(!filename.empty()) { //past file
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
					string  strResponse;
					while(true) { //持续发送，直到成功
					    htool.Post(data_url, strPost, strResponse);
    					if(strResponse.empty()){
#if 1
				        	cout << "response empty" << endl;
#endif
					        sleep(5);
				    	} else {
#if 1
							cout << "stresponse ok" <<endl;
#endif
							pfmgr.update_record();
							break;
						}
					}	
				} else {
					//todo:log
					break;
				}
			}
		} else { //today file
			//todo:if today file exist
			sleep(5);
			cout << "today file processing" << endl;
		}	
	
	}

	cout << "rexmit over" << endl;

	return 0;
}


