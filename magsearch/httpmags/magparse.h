#pragma once

#include "httptool.h"
#include <regex>
#include <string>
#include <queue>
#include <vector>

class MagParse
{
public:
	MagParse();
	~MagParse();

	void parseHTMLLink(char* url);
	void parseMag();
	std::string FormatString(const char * lpcszFormat, ...);
	BOOL UrlEncode(const char* szSrc, char* pBuf, int cbBufLen, BOOL bUpperCase);
	BOOL UrlDecode(const char* szSrc, char* pBuf, int cbBufLen);
	void StringToWstring(std::wstring& szDst, std::string& str);

	std::vector<std::wstring>& getMags();

private:
	std::vector<std::wstring> mags;
};