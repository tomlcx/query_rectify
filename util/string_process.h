#ifndef STRING_PROCESS_H
#define STRING_PROCESS_H

#include <string>
#include <vector>
using namespace std;

inline void trim(string& str, string seperator = " ")
{
	str.erase(0, str.find_first_not_of(seperator));
	str.erase(str.find_last_not_of(seperator) + 1);
}

inline void to_lower(string& str)
{
	for(int i = 0; i < str.size();)
	{
		if(str[i] > 0)
		{
			if(str[i] >= 'A' && str[i] <= 'Z')
				str[i] += 'a' - 'A';
			--i;
		}
		i += 2;
	}
}

inline void to_upper(string& str)
{
	for(int i = 0; i < str.size();)
	{
		if(str[i] > 0)
		{
			if(str[i] >= 'a' && str[i] <= 'z')
				str[i] += 'A' - 'a';
			--i;
		}
		i += 2;
	}
}

inline void delquestionmark(string& str)
{
	if(str.size() == 0)return;
	for(int i = 0; i < str.size() - 1;)
	{
		if(str[i] > 0)
		{
			if(str[i] == '?')
			{
				str.erase(i, 1);
			}
			else
				++i;
		}
		else
		{
			i += 2;
		}
	}
}

inline void split(const string& str, vector<string>& ret, string seperator)
{
        ret.clear();
        if(str.empty())
                return;
        int i = 0, pos = 0;
        while((pos = str.find_first_of(seperator, i)) != string::npos)
        {
                ret.push_back(str.substr(i, pos - i));
                i = pos + 1;
        }
        ret.push_back(str.substr(i));
}

inline bool ifHavModelInTail(const string& str)
{
	for(int i = 0; i < str.size(); ++i)
	{
		if(str[i] > 0)
		{
			if(isalpha(str[i]) && isdigit(str[str.size()-1]))
				return true;
			else
				return false;
		}
		else
			++i;
	}
	return false;
}

inline bool ifHavAlpha(const string& str)
{
	for(int i = 0; i < str.size(); ++i)
	{
		if(str[i] > 0)
		{
			if(isalpha(str[i]))
				return true;
			else
				return false;
		}
		else
			++i;
	}
	return false;
}
#endif
