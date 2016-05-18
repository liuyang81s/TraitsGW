#include <stdio.h>
#include "elog.h"

ElogErrCode init_elog(void) 
{
    /* close printf buffer */
    setbuf(stdout, NULL);
    /* initialize EasyLogger */
	ElogErrCode	ret = elog_init();
    if(ELOG_NO_ERR != ret)
		return ret;

    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO & ~ELOG_FMT_T_INFO);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    
	/* start EasyLogger */
    elog_start();

    return ELOG_NO_ERR;
}

ElogErrCode close_elog(void)
{
	return elog_close();
}

