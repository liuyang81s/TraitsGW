#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defines.h"
#include "traits.h"
#include "httptool.h"
#include "packetfilemgr.h"
#include "traits_elog.h"

using namespace std;

static PacketFileMgr* pfmgr = NULL;
static HttpTool* htool = NULL;

static void sigterm_handler(int sig)
{
    cout << "ReXmit sigterm handler" << endl;

	if(NULL != htool)
		delete htool;
	if(NULL != pfmgr)
		delete pfmgr;

    log_i("ReXmit exit...");
    close_elog();

    exit(EXIT_SUCCESS);
}


int main()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    pid_t pid = fork();
    if(pid < 0) {
        cout << "fork error" << endl;
        exit(EXIT_FAILURE);
    } else if(pid > 0) {
        exit(EXIT_SUCCESS);
    }

    setsid();

    char szPath[1024];
    if(getcwd(szPath, sizeof(szPath)) == NULL)
    {
        cout << "getcwd failed" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        chdir(szPath);
    }

    umask(0);

	signal(SIGTERM, sigterm_handler);

    if(ELOG_NO_ERR != init_elog()) {    
        cout << "ReXmit elog init failed" << endl;                                      
        exit(EXIT_FAILURE);
    }
        
    log_i("ReXmit starting...");

	TRAITScode ret = TRAITSE_LAST;
	string filename;
	string data_url(DEFAULT_SERVER_URL);
	data_url.append(DATA_URL);
	
	pfmgr = new PacketFileMgr;
	if(NULL == pfmgr) {
		log_e("PacketFileMgr alloc failed");
		goto PacketFileMgr_ERROR;
	}
	
    htool = new HttpTool;
	if(NULL == htool) {
		log_e("HttpTool alloc failed");
		goto HttpTool_ERROR;
	}
	
	ret = pfmgr->set_dir(FILEBUF_PATH);		
	if(TRAITSE_OK != ret) {
		goto SETDIR_ERROR;		
	}

	while(true) {
		TRAITS_PF_TIME_TYPE pft_type = TRAITS_PF_INVALID;
		ret = pfmgr->get_file(filename, &pft_type);
		if(TRAITSE_OK != ret) {
			log_e("get past file failed");
			break;//todo: led indication
		}

		if(TRAITS_PF_NONE == pft_type) {
			sleep(5);
			cout << "there is no packet file to process" << endl;
		} else if(TRAITS_PF_PAST == pft_type) {
			while(true) {
				string strPost;
				ret = pfmgr->get_record(strPost);
#if 1
	cout << "strPost=" << strPost << endl;
#endif
				if(TRAITSE_ALL_RECORD_SENT == ret) {
					pfmgr->delete_file(filename);
					break;
				} else if (TRAITSE_OK == ret) {
					if(strPost.empty())
						break;

					string  strResponse;
					while(true) { //持续发送，直到成功
					    htool->Post(data_url, strPost, strResponse);
    					if(strResponse.empty()){
#if 1
				        	cout << "response empty" << endl;
#endif
					        sleep(2);
				    	} else {
#if 1
							cout << "stresponse ok" <<endl;
#endif
							pfmgr->update_record();
							break;
						}
					}	
				} else { //get_record() failed
					log_e("read past file failed");
					break;
				}
			}//while
		} else if(TRAITS_PF_TODAY == pft_type){ //today file
			cout << "today file processing" << endl;
			string strPost;
			ret = pfmgr->get_today_record(strPost);
			if(TRAITSE_OK == ret) {
				if(strPost.empty()) {//读取正常而返回空说明到文件尾
					sleep(5);		 //等待一段时间后再次检查是否有未发送的报文
					continue;
				}
				
				string  strResponse;
				while(true) { //持续发送，直到成功
					htool->Post(data_url, strPost, strResponse);
    				if(strResponse.empty()){
#if 1
			    	cout << "response empty" << endl;
#endif
			       		sleep(2);
				   	} else {
#if 1
						cout << "stresponse ok" <<endl;
#endif
						pfmgr->update_record_today();
						break;
					}
				}//while	
			} else { //get_record() failed
				log_e("read today file failed");
				break;
			}
		}	
	}//while

SETDIR_ERROR:
	if(NULL != htool)
		delete htool;
HttpTool_ERROR:
	if(NULL != pfmgr)
		delete pfmgr;
PacketFileMgr_ERROR:
    log_i("ReXmit exit...");
	close_elog();

    exit(EXIT_SUCCESS);
}


