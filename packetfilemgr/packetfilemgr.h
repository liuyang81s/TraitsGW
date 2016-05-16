#ifndef TRAITS_PACKETFILE_MGR_H
#define TRAITS_PACKETFILE_MGR_H

#include <fstream>
#include <string>
#include <list>

#include "traits.h"

#define PACKET_LINE_SIZE 1024

using namespace std;

class PacketFileMgr
{
public:
	PacketFileMgr();
	virtual ~PacketFileMgr();

	TRAITScode set_dir(const char* dir);
	const char* get_dir();
	
	TRAITScode get_past_file(string& s); 
	TRAITScode delete_file(const string& s); 

	TRAITScode put_record(const string& s);
	TRAITScode get_record(string& s);	
	//mark 'R'->'S'
	TRAITScode update_record();	
	
protected:
	TRAITScode get_today_file_to_put();
	TRAITScode get_today_file_to_get();
	string get_today_name();
	TRAITScode get_file_list();

	string _dir;
	fstream _fs;	
	streampos pos_line_begin;
	streampos pos_line_end;	
	list<string> _filelist;
	char line_buf[PACKET_LINE_SIZE];	
};

#endif

