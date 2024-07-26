#ifndef CODECONVERT_H
#define CODECONVERT_H

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include <vector>
#include <string>
using namespace std;

inline bool code_convert(char* from_charset, char* to_charset, const string& source, string& dest)
{
        iconv_t ct = iconv_open(to_charset, from_charset);
        if(0 == ct || (iconv_t)(-1) == ct)
                return false;
	size_t src_len = source.size();
        size_t dest_len = src_len * 2;
	char* ptr = const_cast<char*>(source.c_str());
	char* buf = new char[dest_len];
	memset(buf, 0, sizeof(char)*dest_len);
	char* ch = buf;
        if((size_t)-1 == iconv(ct, &ptr, &src_len, &ch, &dest_len))
	{
		delete[] buf;
		iconv_close(ct);
                return false;
	}
        iconv_close(ct);
	dest = string(buf, ch);
	delete[] buf;
        return true;
}

inline bool code_convert(char* from_charset, char* to_charset, char* source, char* dest, int destLen)
{
        iconv_t ct = iconv_open(to_charset, from_charset);
        if(0 == ct || (iconv_t)(-1) == ct)
                return false;
	size_t src_len = strlen(source);
        size_t dest_len = destLen;
	char* srcp = source;
	char* destp = dest;
        if((size_t)-1 == iconv(ct, &srcp, &src_len, &destp, &dest_len))
	{
		iconv_close(ct);
                return false;
	}
	dest[destp - dest] = 0;
        iconv_close(ct);
        return true;
}

void GbkToUtf8(char* src,char* out)
{
	size_t inLen = strlen((char*)src);
	size_t outLen = 2*inLen + 1;
	char* inBuf =  (char*)src;
	char* outBuf = (char*)out;
	iconv_t gtou = iconv_open("UTF8","GBK");
	if (gtou == (iconv_t)-1)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s",__FILE__,__LINE__, "iconv init faile");
		return;
	}
	if(iconv(gtou, &inBuf, &inLen, &outBuf, &outLen) == (size_t)-1)
	{
		*out = 0;
		iconv_close(gtou);
		return;
	}
	out[outBuf - (char*)out] = 0;
	iconv_close(gtou);
}

void Utf8ToGbk(char* src,char* out)
{
	size_t inLen = strlen((char*)src);
	size_t outLen = 2*inLen + 1;
	char* inBuf =  (char*)src;
	char* outBuf = (char*)out;
	iconv_t utog = iconv_open("GBK","UTF8");
	if (utog == (iconv_t)-1)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s",__FILE__,__LINE__, "iconv init faile");
		return;
	}
	if(iconv(utog, &inBuf, &inLen, &outBuf, &outLen) == (size_t)-1)
	{
		*out = 0;
		iconv_close(utog);
		return;
	}
	out[outBuf - (char*)out] = 0;
	iconv_close(utog);
}

#endif
