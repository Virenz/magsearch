#include "magparse.h"

#define DEFAULT_PAGE_BUF_SIZE 1048576

std::queue<std::string> hrefUrl;

MagParse::MagParse()
{
}

MagParse::~MagParse()
{
	
}

std::vector<std::string>& MagParse::getMags()
{
	// TODO: 在此处插入 return 语句
	return this->mags;
}

void MagParse::parseHTMLLink(char * url)
{
	char* host = "http://www.ciliba.org";
	int len = 0;
	char pBuf[256];
	BOOL isSucess = UrlEncode(url, pBuf, 256, true);
	if (!isSucess)
	{
		return;
	}
	//%E6%88%98%E7%8B%BC
	std::string searcher_url = FormatString("%s/s/%s.html", host, pBuf);

	HttpTool* httpTool = new HttpTool();
	bool isGet = httpTool->httpGet(searcher_url.c_str());
	if (!isGet)
		return;
	std::string html_content(httpTool->getReponseHTML());
	delete httpTool;
	
	// 正则表达获取网址
	int index = 0;
	const std::regex pattern("http://www.ciliba.org/detail/[0-9a-z]{40}.html");
	for (std::sregex_iterator it(html_content.begin(), html_content.end(), pattern), end;     //end是尾后迭代器，regex_iterator是regex_iterator的string类型的版本
		it != end;
		++it, index++)
	{
		hrefUrl.push(it->str());
	}
}

void MagParse::parseMag()
{
	while (hrefUrl.size() != 0) {
		std::string url = hrefUrl.front();  // 从队列的最开始取出一个网址
		
		HttpTool* httpTool = new HttpTool();
		bool isGet = httpTool->httpGet(url.c_str());
		if (!isGet)
			return;
		std::string magHtml(httpTool->getReponseHTML());
		delete httpTool;

		// 正则表达获取网址

		const std::regex pattern("([\u4e00-\u9fa5]+)");
		//const std::regex pattern("</div><ol><li>([\\u4e00-\\u9fa5]+)<span class=\"cpill blue-pill\">([\\.\\d]{3,} \\w{2})</span>");
		//const std::regex pattern("href=\"(magnet.*\\w{40})");
		std::smatch result;
		bool valid = std::regex_search(magHtml, result, pattern);
		//此处result参数可有可无，result是一个字符串数组，用来存储正则表达式里面括号的内容。
		std::string strbuf;
		if(valid&&(result.length()>0))
		{
			for(int i = 1;i<result.size();i++)
			{
				strbuf.append(result[i]);
				strbuf.append("\t");
				//mags.push_back(result[i]);
			}
		}
		mags.push_back(strbuf);
		strbuf.clear();
		hrefUrl.pop();                 // 遍历完之后，删除这个网址
	}
}

std::string MagParse::FormatString(const char * lpcszFormat, ...)
{
	char *pszStr = NULL;
	if (NULL != lpcszFormat)
	{
		va_list marker = NULL;
		va_start(marker, lpcszFormat); //初始化变量参数  
		size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
		pszStr = new char[nLength];
		memset(pszStr, '\0', nLength);
		_vsnprintf_s(pszStr, nLength, nLength, lpcszFormat, marker);
		va_end(marker); //重置变量参数  
	}
	std::string strResult(pszStr);
	delete[]pszStr;
	return strResult;
}

BOOL MagParse::UrlEncode(const char* szSrc, char* pBuf, int cbBufLen, BOOL bUpperCase)
{
	if (szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return FALSE;

	size_t len_ascii = strlen(szSrc);
	if (len_ascii == 0)
	{
		pBuf[0] = 0;
		return TRUE;
	}

	//先转换到UTF-8
	char baseChar = bUpperCase ? 'A' : 'a';
	int cchWideChar = MultiByteToWideChar(CP_ACP, 0, szSrc, len_ascii, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc((cchWideChar + 1) * sizeof(WCHAR));
	if (pUnicode == NULL)
		return FALSE;
	MultiByteToWideChar(CP_ACP, 0, szSrc, len_ascii, pUnicode, cchWideChar + 1);

	int cbUTF8 = WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, NULL, 0, NULL, NULL);
	LPSTR pUTF8 = (LPSTR)malloc((cbUTF8 + 1) * sizeof(CHAR));
	if (pUTF8 == NULL)
	{
		free(pUnicode);
		return FALSE;
	}
	WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, pUTF8, cbUTF8 + 1, NULL, NULL);
	pUTF8[cbUTF8] = '\0';

	unsigned char c;
	int cbDest = 0; //累加
	unsigned char *pSrc = (unsigned char*)pUTF8;
	unsigned char *pDest = (unsigned char*)pBuf;
	while (*pSrc && cbDest < cbBufLen - 1)
	{
		c = *pSrc;
		if (isalpha(c) || isdigit(c) || c == '-' || c == '.' || c == '~')
		{
			*pDest = c;
			++pDest;
			++cbDest;
		}
		else if (c == ' ')
		{
			*pDest = '+';
			++pDest;
			++cbDest;
		}
		else
		{
			//检查缓冲区大小是否够用？
			if (cbDest + 3 > cbBufLen - 1)
				break;
			pDest[0] = '%';
			pDest[1] = (c >= 0xA0) ? ((c >> 4) - 10 + baseChar) : ((c >> 4) + '0');
			pDest[2] = ((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + baseChar) : ((c & 0xF) + '0');
			pDest += 3;
			cbDest += 3;
		}
		++pSrc;
	}
	//null-terminator
	*pDest = '\0';
	free(pUnicode);
	free(pUTF8);
	return TRUE;
}

//解码后是utf-8编码
BOOL MagParse::UrlDecode(const char* szSrc, char* pBuf, int cbBufLen)
{
	if (szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return FALSE;

	size_t len_ascii = strlen(szSrc);
	if (len_ascii == 0)
	{
		pBuf[0] = 0;
		return TRUE;
	}

	char *pUTF8 = (char*)malloc(len_ascii + 1);
	if (pUTF8 == NULL)
		return FALSE;

	int cbDest = 0; //累加
	unsigned char *pSrc = (unsigned char*)szSrc;
	unsigned char *pDest = (unsigned char*)pUTF8;
	while (*pSrc)
	{
		if (*pSrc == '%')
		{
			*pDest = 0;
			//高位
			if (pSrc[1] >= 'A' && pSrc[1] <= 'F')
				*pDest += (pSrc[1] - 'A' + 10) * 0x10;
			else if (pSrc[1] >= 'a' && pSrc[1] <= 'f')
				*pDest += (pSrc[1] - 'a' + 10) * 0x10;
			else
				*pDest += (pSrc[1] - '0') * 0x10;

			//低位
			if (pSrc[2] >= 'A' && pSrc[2] <= 'F')
				*pDest += (pSrc[2] - 'A' + 10);
			else if (pSrc[2] >= 'a' && pSrc[2] <= 'f')
				*pDest += (pSrc[2] - 'a' + 10);
			else
				*pDest += (pSrc[2] - '0');

			pSrc += 3;
		}
		else if (*pSrc == '+')
		{
			*pDest = ' ';
			++pSrc;
		}
		else
		{
			*pDest = *pSrc;
			++pSrc;
		}
		++pDest;
		++cbDest;
	}
	//null-terminator
	*pDest = '\0';
	++cbDest;

	int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc(cchWideChar * sizeof(WCHAR));
	if (pUnicode == NULL)
	{
		free(pUTF8);
		return FALSE;
	}
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, pUnicode, cchWideChar);
	WideCharToMultiByte(CP_ACP, 0, pUnicode, cchWideChar, pBuf, cbBufLen, NULL, NULL);
	free(pUTF8);
	free(pUnicode);
	return TRUE;
}