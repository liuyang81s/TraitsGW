#ifndef TRAITS_DEFINES_H
#define TRAITS_DEFINES_H

#define TRAITS_DEBUG
#define TRAITS_DEBUG_GW
//#define TRAITS_DEBUG_HB
#define TRAITS_DEBUG_SERIAL

#define TRAITS_TEST

//#define DEBUG_ON_PC

#ifndef DEBUG_ON_PC
#define FILEBUF_PATH	"/mnt/sda1/filebuf/" 
#define ELOG_PATH		"/mnt/sda1/logs/"
#else
#define FILEBUF_PATH    "/home/ayang/test/filebuf/"
#define ELOG_PATH		"/home/ayang/test/logs/"
#endif

#endif

