#ifndef WHOLE_PINYIN_CREATE_H
#define WHOLE_PINYIN_CREATE_H

#include <set>
#include <map>
#include <tr1/unordered_map>
#include "pinyin_process.h"
#include "edit_distance.h"
#include "index_create.h"
#include "BitMap.h"

class WholePinYinCreator
{
public:
	void init(map<string, string>& config);
	bool load();
	//bool create(tr1::unordered_map<string, int>& wpy);
	bool create(map<string, int>& wpy);
	bool rectify(string& ori_word, vector<pair<string, int> >& rec_word);
	bool save();
private:
	int m_ed;
	static const int MAXWORDLEN = 25;
	IndexCreator idx_create;
};

#endif
