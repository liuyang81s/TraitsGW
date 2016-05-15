#include <iostream>
#include <unistd.h>
#include <string>

#include "traits.h"
#include "http.h"
#include "packetfilemgr.h"

using namespace std;

int main()
{
	PacketFileMgr pfmgr;
	string filename;
    HttpTool htool;
	string data_url = SERVER_URL + DATA_URL;
	TRAITScode ret = TRAITSE_LAST;
	
	pfmgr.set_dir(FILEBUF_ADDR);		

	while(true) {
		ret = pfmgr.get_past_file(filename);
		if(TRAITSE_OK != ret)
			break;//todo: log, led indication
	
		if(!filename.empty()) { //past file
			while(true) {
				string strPost;
				ret = pfmgr.get_record(line);
				if(TRAITSE_ALL_RECORD_SENT == ret) {
					pfmgr.delete_file(filename);
					break;
				} else if (TRAITSE_OK == ret) {
					string  strResponse;
					while(true) { //持续发送，直到成功
					    htool.Post(data_url, strPost, strResponse);
    					if(strResponse.empty()){
#ifdef TRAITS_DEBUG_GW
				        	cout << "response empty" << endl;
#endif
	        				//todo:log
					        sleep(5);
				    	} else 
							break;
					}	
				} else {
					//todo:log
					break;
				}
			}
		} else { //today file
			
		}	
	
	}



	cout << "main thread exit" << endl;

	return 0;
}


