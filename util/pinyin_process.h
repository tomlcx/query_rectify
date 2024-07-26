#ifndef PINYIN_PROCESS_H
#define PINYIN_PROCESS_H

#include <set>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

inline bool getFirstPinYin(const string& chword, int pos, string str, map<string, set<string> >& chword_pinyin, vector<string>& first_pinyin)
{
	if(pos >= chword.size())
	{
		first_pinyin.push_back(str);
		return true;
	}
	string word = chword.substr(pos, 2);
	map<string, set<string> >::iterator sSIt;
	sSIt = chword_pinyin.find(word);
	if(sSIt != chword_pinyin.end())
	{
		if(sSIt->second.size() == 0)
			return false;
		set<string>& py = sSIt->second;
		for(set<string>::iterator sIt = py.begin(); sIt != py.end(); ++sIt)
		{
			if(!getFirstPinYin(chword, pos + 2, str+(*sIt)[0], chword_pinyin, first_pinyin))
				return false;
		}
	}
	else
		return false;
	return true;
}

inline bool getWholePinYin(const string& chword, int pos, vector<string>& vec, 
	map<string, set<string> >& chword_pinyin, vector<vector<string> >& whole_pinyin)
{
	if(pos >= chword.size())
	{
		whole_pinyin.push_back(vec);
		return true;
	}
	string word = chword.substr(pos, 2);
	map<string, set<string> >::iterator sSIt;
	sSIt = chword_pinyin.find(word);
	if(sSIt != chword_pinyin.end())
	{
		if(sSIt->second.size() == 0)
			return false;
		set<string>& py = sSIt->second;
		for(set<string>::iterator sIt = py.begin(); sIt != py.end(); ++sIt)
		{
			vec.push_back(*sIt);
			if(!getWholePinYin(chword, pos + 2, vec, chword_pinyin, whole_pinyin))
				return false;
			vec.pop_back();
		}
	}
	else
		return false;
	return true;
}

inline bool getWholePinYin(const string& chword, int pos, string str, 
	map<string, set<string> >& chword_pinyin, vector<string>& whole_pinyin)
{
	if(pos >= chword.size())
	{
		whole_pinyin.push_back(str);
		return true;
	}
	string word = chword.substr(pos, 2);
	map<string, set<string> >::iterator sSIt;
	sSIt = chword_pinyin.find(word);
	if(sSIt != chword_pinyin.end())
	{
		if(sSIt->second.size() == 0)
			return false;
		set<string>& py = sSIt->second;
		for(set<string>::iterator sIt = py.begin(); sIt != py.end(); ++sIt)
		{
			if(!getWholePinYin(chword, pos + 2, str + *sIt, chword_pinyin, whole_pinyin))
				return false;
		}
	}
	else
		return false;
	return true;
}

inline bool getPinYin(const string& chword, int pos, vector<string>& vec, string str, 
				map<string, set<string> >& chword_pinyin, 
				vector<vector<string> >& whole_pinyin, vector<string>& first_pinyin)
{
	if(pos >= chword.size())
	{
		whole_pinyin.push_back(vec);
		first_pinyin.push_back(str);
		return true;
	}
	string word = chword.substr(pos, 2);
	map<string, set<string> >::iterator sSIt;
	sSIt = chword_pinyin.find(word);
	if(sSIt != chword_pinyin.end())
	{
		if(sSIt->second.size() == 0)
			return false;
		set<string>& py = sSIt->second;
		for(set<string>::iterator sIt = py.begin(); sIt != py.end(); ++sIt)
		{
			vec.push_back(*sIt);
			if(!getPinYin(chword, pos + 2, vec, str+(*sIt)[0], chword_pinyin, whole_pinyin, first_pinyin))
				return false;
			vec.pop_back();
		}
	}
	else
		return false;
	return true;
}

inline bool getPinYin(const string& chword, map<string, set<string> >& chword_pinyin, 
		vector<vector<string> >& whole_pinyin, vector<string>& first_pinyin)
{
	whole_pinyin.clear();
	first_pinyin.clear();
	vector<string> wpy;
	string str;
	if(!getPinYin(chword, 0, wpy, str, chword_pinyin, whole_pinyin, first_pinyin))
	{
		whole_pinyin.clear();
		first_pinyin.clear();
		return false;
	}
	return true;
}

inline bool getWholePinYin(const string& chword, map<string, set<string> >& chword_pinyin,
		vector<vector<string> >& whole_pinyin)
{
	whole_pinyin.clear();
	vector<string> wpy;
	if(!getWholePinYin(chword, 0, wpy, chword_pinyin, whole_pinyin))
	{
		whole_pinyin.clear();
		return false;
	}
	return true;
}

inline bool getWholePinYin(const string& chword, map<string, set<string> >& chword_pinyin,
		vector<string>& whole_pinyin)
{
	whole_pinyin.clear();
	string wpy;
	if(!getWholePinYin(chword, 0, wpy, chword_pinyin, whole_pinyin))
	{
		whole_pinyin.clear();
		return false;
	}
	return true;
}

inline bool getFirstPinYin(const string& chword, map<string, set<string> >& chword_pinyin, 
					vector<string>& first_pinyin)
{
	first_pinyin.clear();
	string str;
	if(!getFirstPinYin(chword, 0, str, chword_pinyin, first_pinyin))
	{
		first_pinyin.clear();
		return false;
	}
	return true;
}

#endif
