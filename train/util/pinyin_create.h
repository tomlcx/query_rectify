#ifndef PINYIN_CREATE_H
#define PINYIN_CREATE_H

#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <map>
#include <tr1/unordered_map>
#include "edit_distance.h"
#include "index_create.h"
using namespace std;

class PinYinCreator
{
public:
	void init(const string& out_path, map<string, string>& config);
	bool load();
	bool create(tr1::unordered_map<string, int>& chword, map<string, set<string> >& chword_pinyin);
	bool rectify(const string& first_pinyin, const string& ori_word, vector<pair<string, int> >& rec_word);
	bool getPinYin(const string& chword, map<string, set<string> >& chword_pinyin, 
		vector<vector<string> >& whole_pinyin, vector<string>& first_pinyin);
	bool getPinYin(const string& chword, int pos, vector<string>& vec, string str, map<string, set<string> >& chword_pinyin, 
			vector<vector<string> >& whole_pinyin, vector<string>& first_pinyin);
	bool getFirstPinYin(const string& chword, map<string, set<string> >& chword_pinyin, vector<string>& first_pinyin);
	bool getFirstPinYin(const string& chword, int pos, string str, map<string, set<string> >& chword_pinyin, vector<string>& first_pinyin);
	bool save();
private:
	int m_ed;
	static const int MAXWORDLEN = 8;
	IndexCreator idx_create;
};

#endif
