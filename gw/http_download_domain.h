
#ifndef __HTTP_DOWNLOAD_DOMAIN
#define __HTTP_DOWNLOAD_DOMAIN

#include <string>
#include "curl/curl.h"

class HttpDownloadDomain
{
public:
	HttpDownloadDomain(bool* cancel);
	~HttpDownloadDomain();
	    /** 
    * @brief HTTP POST请求 
    * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com 
    * @param strPost 输入参数,使用如下格式para1=val1¶2=val2&… 
    * @param strResponse 输出参数,返回的内容 
    * @return 返回是否Post成功 
    */  
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse); 
	bool *cancel_;
};

#endif