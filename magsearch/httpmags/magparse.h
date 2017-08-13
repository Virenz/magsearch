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

	std::vector<std::string>& getMags();

private:
	std::vector<std::string> mags;
};