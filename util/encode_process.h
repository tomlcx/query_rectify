#ifndef ENCODE_PROCESS_H
#define ENCODE_PROCESS_H
#include <iconv.h>
#include <vector>
using namespace std;

enum CHARTYPE
{
	ENWORD,
	CHWORD,
	ENPUNC,
	CHPUNC,
	ALPHA,
	NUMBER
};

//ÊÇ·ñÊÇGBKºº×Ö
inline bool isGBKCH(unsigned char ch1, unsigned char ch2)
{
	return (ch2 >= 0x40 && ch2 <= 0xFE && ch2 != 0x7F) && 
		(((ch1 >= 0x81 && ch1 <= 0xA0) || (ch1 >= 0xB0 && ch1 <= 0xF7)) ||
		(((ch1 >= 0xAA && ch1 <= 0xAF) || (ch1 >= 0xF8 && ch1 <= 0xFE)) && ch2 <= 0xA0));
}

inline bool isGBKCH(const string& str)
{
	return str.size() == 2 && isGBKCH(str[0], str[1]);
}

inline bool encode_convert(const string& source, string from_code, string to_code, string& dest)
{
	iconv_t cv = iconv_open(to_code.c_str(), from_code.c_str());
	if(cv == 0 || cv == (iconv_t)(-1))
		return false;
	size_t source_len = source.size();
	size_t buf_len = source_len * 2;
	char* buf = new char[buf_len];
	if(!buf)
		return false;
	memset(buf, 0, sizeof(char)*buf_len);
	char* ptr = const_cast<char*>(source.c_str());
	char* ch = buf;
	if((size_t)(-1) == iconv(cv, &ptr, &source_len, &ch, &buf_len))
	{
		delete[] buf;
		iconv_close(cv);
		return false;
	}
	iconv_close(cv);
	dest = string(buf, buf+strlen(buf));
	delete[] buf;
	return true;
}

inline void splitCHEN(const string& str, vector<string>& vecWord, vector<CHARTYPE>& vecMark, bool distin = false)
{
	vecWord.clear();
	vecMark.clear();
	CHARTYPE ct, pre_ct;
	string tmp;
	int ch_or_en;
	for(int i = 0; i < str.size();)
	{
		if(str[i] > 0)
		{
			ch_or_en = 1;
			if(isalnum(str[i]) || 
				(str[i] == '.' && 
				(!distin || 
				(i > 0 && i + 1 < str.size() && isdigit(str[i-1]) && isdigit(str[i+1])))))
				ct = ENWORD;
			else
				ct = ENPUNC;
		}
		else
		{
			if(i + 1 >= str.size())
				break;
			ch_or_en = 2;
			if(isGBKCH(str[i], str[i+1]))
				ct = CHWORD;
			else
				ct = CHPUNC;
		}
		if(0 != i && pre_ct != ct)
		{
			vecWord.push_back(tmp);
			vecMark.push_back(pre_ct);
			tmp = "";
		}
		for(int n = 0; n < ch_or_en; ++n)
			tmp += str[i+n];
		pre_ct = ct;
		i += ch_or_en;
	}
	if(tmp != "")
	{
		vecWord.push_back(tmp);
		vecMark.push_back(ct);
	}
}

inline void splitCHAlNum(const string& str, vector<string>& vecWord, vector<CHARTYPE>& vecMark, bool distin = false)
{
	vecWord.clear();
	vecMark.clear();
	CHARTYPE ct, pre_ct;
	string tmp;
	int ch_or_en;
	for(int i = 0; i < str.size();)
	{
		if(str[i] > 0)
		{
			ch_or_en = 1;
			if(isalpha(str[i]))
				ct = ALPHA;
			else if(isdigit(str[i]) || 
				(str[i] == '.' && i > 0 && isdigit(str[i-1]) &&
				(!distin || 
				(i + 1 < str.size() && isdigit(str[i+1])))))
				ct = NUMBER;
			else
				ct = ENPUNC;
		}
		else
		{
			if(i + 1 >= str.size())
				break;
			ch_or_en = 2;
			if(isGBKCH(str[i], str[i+1]))
				ct = CHWORD;
			else
				ct = CHPUNC;
		}
		if(0 != i && pre_ct != ct)
		{
			vecWord.push_back(tmp);
			vecMark.push_back(pre_ct);
			tmp = "";
		}
		for(int n = 0; n < ch_or_en; ++n)
			tmp += str[i+n];
		pre_ct = ct;
		i += ch_or_en;
	}
	if(tmp != "")
	{
		vecWord.push_back(tmp);
		vecMark.push_back(ct);
	}
}
#endif
