#pragma once

#include <string>
#include "../curl/curl.h"
#pragma comment(lib, "libcurl_a.lib") 

typedef struct MemoryStruct {
	char *memory;
	size_t size;
}_MEMORYSTRUNCT;

class HttpTool
{
public:
	HttpTool();
	~HttpTool();

	bool httpGet(const char* sourses);
	bool httpPost(const char* url, const char* strpost);
	char* getReponseHTML();
	void cleanChunk();

private:
	_MEMORYSTRUNCT chunk;
};