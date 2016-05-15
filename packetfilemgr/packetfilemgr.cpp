#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

#include "packetfilemgr.h"
#include "traits.h"


PacketFileMgr::PacketFileMgr()
{
	_dir.clear();
	_filelist.clear();
}

PacketFileMgr::~PacketFileMgr()
{
	
}

TRAITScode PacketFileMgr::set_dir(const char* dir)
{
	if(NULL == dir)
		return TRAITSE_ARG_INVALID;

	_dir = dir;
	
	if(0 == mkdir(_dir.c_str(), 0755))
		return TRAITSE_OK;
	else {
		if(EEXIST == errno)
			return TRAITSE_OK;
		else
			return TRAITSE_DIR_CREATE_FAILED; 	
	}
}

const char* PacketFileMgr::get_dir() 
{
	return _dir.c_str();
}

TRAITScode PacketFileMgr::put_record(const string& s)
{
	
	_fs.close();

	return TRAITSE_OK;
}

TRAITScode PacketFileMgr::get_record(string& s)
{
	bool ALL_SENT = true;

	while(!_fs.eof()) {
		memset(line_buf, 0, PACKET_LINE_SIZE);
		pos_line_begin = _fs.tellp();
		_fs.getline(line_buf, PACKET_LINE_SIZE);
		pos_line_end = _fs.tellp();
		if(_fs.bad()) {
			_fs.close();
			return TRAITSE_FILE_READ_ERROR;
		}
		if('S' == line_buf[0])
			continue;
		else if('R' == line_buf[0]) {
			s.assign(line_buf + 2);
			ALL_SENT = false;
			break;
		} else {
			//todo: log invalid record
			continue;
		}
	}

	if(ALL_SENT == true) {
		_fs.close();
		return TRAITSE_ALL_RECORD_SENT;
	}
	else
		return TRAITSE_OK;
}

TRAITScode PacketFileMgr::update_record()
{
	_fs.seekp(pos_line_begin);
	_fs.put('S');	//R -> S
	_fs.flush();
	_fs.seekp(pos_line_end);

	return TRAITSE_OK;
}

TRAITScode PacketFileMgr::get_today_file()
{
	time_t cur_t;
	time(&cur_t);
	
	char day[16];
	memset(day, 0, 16);

	strftime(day, 16, "%Y-%m-%d", localtime(&cur_t));

	string filename = _dir + day; 
#if 1
		cout << "filename = " << filename << endl;
#endif

	_fs.open(filename.c_str(), ios::in | ios::app | ios::binary);  	

    if (_fs.is_open())
		return TRAITSE_OK;
    else{
		_fs.open(filename.c_str(), ios::in | ios::out | ios::binary);  	
		if(_fs.is_open()) 
			return TRAITSE_OK;
		else {
			;//todo: log, and error indication
			return TRAITSE_FILE_CREATE_FAILED;
		}
	}
}


TRAITScode PacketFileMgr::get_file_list()
{
    DIR *dir;
    struct dirent entry;
    struct dirent *res;
	
	_filelist.clear();

    if ((dir = opendir(_dir.c_str())) == NULL) { //打开目录
		//todo: log
		return TRAITSE_DIR_OPEN_FAILED;
	} else {
    	int ret;
		for (ret = readdir_r(dir, &entry, &res); 
			res != NULL && ret == 0;
			ret = readdir_r(dir, &entry, &res)) {
            if (entry.d_type != DT_DIR) {//存放到列表中
                _filelist.push_back(string(entry.d_name));
            }   
        }   
        closedir(dir);
    }   
#if 1
	cout << "file list: " << endl;
    for(list<string>::iterator it = _filelist.begin(); it != _filelist.end(); it++)
    {   
        cout<<(*it)<<endl;
    }
#endif 
	return TRAITSE_OK;
}

TRAITScode PacketFileMgr::get_past_file(string& s) 
{
	TRAITScode ret = get_file_list();
	if(TRAITSE_OK != ret)
		return ret;
	else {
		time_t cur_t;
		time(&cur_t);
	
		char today[16];
		memset(today, 0, 16);
		strftime(today, 16, "%Y-%m-%d", localtime(&cur_t));

		s.clear();
    	for(list<string>::iterator it = _filelist.begin(); it != _filelist.end(); it++)
	    {   
    	    if((*it) != today) {
				s = (*it);
				break;
			}				
	    }

		//s空，说明没有过去的遗留缓存文件
		if(s.empty())
			return TRAITSE_OK;
#if 1
		cout << "past file name = " << s << endl;
#endif

		if(_fs.is_open())
			_fs.close();	

		string filepath = _dir + s;
		_fs.open(filepath.c_str(), ios::in | ios::out | ios::binary);  	
		if(_fs.is_open()) 
			return TRAITSE_OK;
		else {
			;//todo: log, and error indication
			return TRAITSE_FILE_OPEN_FAILED;
		}
	}
}

TRAITScode PacketFileMgr::delete_file(const string& file)
{
	string path = _dir + file;

	if(!remove(path.c_str()))
		return TRAITSE_OK;
	else {
		//todo: log
		return TRAITSE_FILE_DELETE_FAILED;
	}
}


