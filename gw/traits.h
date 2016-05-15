#ifndef _TRAITS_TRAITS_H
#define _TRAITS_TRAITS_H

/* All possible error codes from all sorts of traits gw functions. Future versions
   may return other values, stay prepared.

   Always add new return codes last. Never *EVER* remove any. The return
   codes must remain the same!
*/
typedef enum {
	TRAITSE_OK = 0,
	TRAITSE_ARG_INVALID,
	TRAITSE_CONFIG_FILE_NOT_FOUND,
	TRAITSE_FILE_CREATE_FAILED,
	TRAITSE_FILE_DELETE_FAILED,
	TRAITSE_FILE_OPEN_FAILED,
	TRAITSE_FILE_READ_ERROR,
	TRAITSE_FILE_WRITE_ERROR,
	TRAITSE_DIR_CREATE_FAILED,
	TRAITSE_DIR_OPEN_FAILED,
	TRAITSE_CONFIG_PARAM_NOT_FOUND,
	TRAITSE_MAC_NOT_FOUND,
	TRAITSE_RESPONSE_NONE,
	TRAITSE_RESPONSE_FORMAT_ERROR,
	TRAITSE_RESPONSE_CONTENT_ERROR,
	TRAITSE_MEM_ALLOC_FAILED,
	TRAITSE_TIME_FORMAT_ERROR,
	TRAITSE_TIME_EXPIRED,
	TRAITSE_ALL_RECORD_SENT,
	TRAITSE_LAST /* never use! */
} TRAITScode;


#define SERVER_URL "http://traits.imwork.net:10498/AnalyzeServer/system/"
#define INIT_URL "init.do"
#define DATA_URL "data.do"
#define HB_URL	"refresh.do"

#if 0
#define FILEBUF_ADDR	"/mnt/sda1/filebuf/" 
#else
#define FILEBUF_ADDR	"/home/ayang/test/filebuf/"
#endif

#endif

