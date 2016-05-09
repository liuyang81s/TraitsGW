#ifndef _TRAITS_TRAITS_H
#define _TRAITS_TRAITS_H

/* All possible error codes from all sorts of traits gw functions. Future versions
   may return other values, stay prepared.

   Always add new return codes last. Never *EVER* remove any. The return
   codes must remain the same!
*/
typedef enum {
	TRAITSE_OK = 0,
	TRAITSE_CONFIG_FILE_NOT_FOUND,
	TRAITSE_CONFIG_PARAM_NOT_FOUND,
	TRAITSE_MAC_NOT_FOUND,
	TRAITSE_RESPONSE_NONE,
	TRAITSE_RESPONSE_FORMAT_ERROR,
	TRAITSE_RESPONSE_CONTENT_ERROR,
	TRAITSE_MEM_ALLOC_FAILED,
	TRAITSE_LAST /* never use! */
} TRAITScode;

#endif

