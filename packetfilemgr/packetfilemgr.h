#ifndef TRAITS_PACKETFILE_MGR_H
#define TRAITS_PACKETFILE_MGR_H

#include <fstream>
#include <string>
#include <list>
#include <stdio.h>

#include "traits.h"

#define PACKET_LINE_SIZE 1024

using namespace std;

typedef enum{
	TRAITS_PF_NONE,		//have no packet file
	TRAITS_PF_PAST,		//packet file of the day before exists
	TRAITS_PF_TODAY,	//packet file of today exists
	TRAITS_PF_INVALID,
}TRAITS_PF_TIME_TYPE;


class PacketFileMgr
{
public:
	PacketFileMgr();
	virtual ~PacketFileMgr();

	TRAITScode set_dir(const char* dir);
	const char* get_dir();
	
	TRAITScode get_file(string& s, TRAITS_PF_TIME_TYPE* pft_type); 
	TRAITScode delete_file(const string& s); 

	TRAITScode get_record(string& s);	
	TRAITScode put_today_record(const string& s);
	TRAITScode get_today_record(string& s);	
	//mark 'R'->'S'
	TRAITScode update_record();
	TRAITScode update_record_today();
	
protected:
	TRAITScode get_today_file_to_put();
	TRAITScode get_today_file_to_get();
	string get_today_name();
	TRAITScode get_file_list();

	string _dir;

	fstream _fs;	
	streampos pos_line_begin;
	streampos pos_line_end;
	
	FILE* _fp;
	fpos_t pos_line_begin_todayfile;
	fpos_t pos_line_end_todayfile;

	list<string> _filelist;
	char line_buf[PACKET_LINE_SIZE];	
};

#endif

