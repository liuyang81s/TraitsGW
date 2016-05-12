#include <curl/curl.h>
#include "httptool.h"

#define POST_TIMEOUT 10

HttpTool::HttpTool()
{
}

HttpTool::~HttpTool()
{
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);  
	if( NULL == str || NULL == buffer )  
	{  
		return -1;  
	}  

	char* pData = (char*)buffer;  
	str->append(pData, size * nmemb);  
	return nmemb;  
}  

int HttpTool::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
	CURLcode res;  
	struct curl_slist *headerlist=NULL;
	static const char buf[] = "Expect:";
	
	CURL* curl = curl_easy_init(); 
	if(NULL == curl)  
	{  
		return CURLE_FAILED_INIT;  
	}  

	headerlist = curl_slist_append(headerlist, buf);
	curl_slist_append(headerlist, "Content-Type:application/json;charset=UTF-8");//clientTag
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
	curl_easy_setopt(curl, CURLOPT_POST, 1);  
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, POST_TIMEOUT);  
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, POST_TIMEOUT);  

	res = curl_easy_perform(curl);  
	curl_easy_cleanup(curl);  
	
	return res;  
} 
 
