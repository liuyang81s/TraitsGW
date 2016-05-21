/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for linux.
 * Created on: 2015-04-28
 */

#include <elog.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>       
#include <sys/types.h>

#include "defines.h"

static pthread_mutex_t output_lock;
static FILE* fp;

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    pthread_mutex_init(&output_lock, NULL);

#ifdef ELOG_TARGET_STDOUT
	fp = stdout;
#else
    if(0 != mkdir(LOG_PATH, 0755)) {
        if(EEXIST != errno) {
			perror(strerror(errno));
            return ELOG_DIR_CREATE_ERROR;
		}
    } 

	time_t cur_t;
	time(&cur_t);

	char cur[16];
    memset(cur, 0, 16);

	strftime(cur, 16, "%Y%m%d%H%M%S", localtime(&cur_t));

	char filepath[64];
	memset(filepath, 0, 64);

	int i = strlen(LOG_PATH);
	strncpy(filepath, LOG_PATH, i);
	strncpy(filepath + i, cur, strlen(cur));
	i += strlen(cur);
	strncpy(filepath + i, ".log", strlen(".log"));
	
    fp = fopen(filepath, "w+");
    if(NULL == fp) {
		perror(strerror(errno));
        result = ELOG_FILE_OPEN_ERROR;    
    }   
#endif
    
    return result;
}

ElogErrCode elog_port_close(void) {
    ElogErrCode result = ELOG_NO_ERR;

    pthread_mutex_destroy(&output_lock);

#ifndef ELOG_TARGET_STDOUT
    fclose(fp);
#endif

    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    fprintf(fp, "%.*s", (int)size, log);
    fflush(fp);
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    pthread_mutex_lock(&output_lock);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    pthread_mutex_unlock(&output_lock);
}


/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char cur_system_time[24] = { 0 };
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);
    if (p == NULL) {
        return "";
    }
    snprintf(cur_system_time, 18, "%02d-%02d %02d:%02d:%02d", p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);

    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    static char cur_process_info[10] = { 0 };

    snprintf(cur_process_info, 10, "pid:%04d", getpid());

    return cur_process_info;
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    static char cur_thread_info[10] = { 0 };

    snprintf(cur_thread_info, 10, "tid:%04ld", pthread_self());

    return cur_thread_info;
}
