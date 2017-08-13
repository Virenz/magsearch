#include "httptool.h"
#include <commctrl.h>

HttpTool::HttpTool()
{
	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */
}

HttpTool::~HttpTool()
{
	if (chunk.memory)
	{
		free(chunk.memory);
		chunk.memory = NULL;
	}
	chunk.size = 0;
}

size_t WriteData(void* ptr, size_t size, size_t nmemb, void* stream)
{
	size_t realsize = size * nmemb;
	_MEMORYSTRUNCT* mem = (_MEMORYSTRUNCT*)stream;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

bool HttpTool::httpGet(const char * sourses)
{
	bool isSuccess = false;
	long http_response_code = 0;

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	/* initalize custom header list (stating that Expect: 100-continue is not
	wanted */

	if (curl) {
		/* what URL that receives this GET */
		curl_easy_setopt(curl, CURLOPT_URL, sourses);

		//通过write_data方法将联网返回数据写入到data中
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		/* Perform the request, res will get the return code */

		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			isSuccess = false;
		}
		else
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
			if (http_response_code == 200)
			{
				printf("%s memory retrieved\n", chunk.memory);
				isSuccess = true;
			}	
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return  isSuccess;
}

bool HttpTool::httpPost(const char * url, const char * strpost)
{
	bool isSuccess = false;
	long http_response_code = 0;

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	/* initalize custom header list (stating that Expect: 100-continue is not
	wanted */

	if (curl) {
		/* what URL that receives this POST */

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strpost);

		//通过write_data方法将联网返回数据写入到data中
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		/**
		* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
		* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
		*/
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

		/* Perform the request, res will get the return code */

		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			isSuccess = false;
		}
		else
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
			if (http_response_code == 200)
			{
				printf("%s memory retrieved\n", chunk.memory);
				isSuccess = true;
			}
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return  isSuccess;
}

char* HttpTool::getReponseHTML()
{
	return this->chunk.memory;
}

void HttpTool::cleanChunk()
{
	free(chunk.memory);
	chunk.memory = NULL;
	chunk.size = 0;
}
