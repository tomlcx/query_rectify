#ifndef CHWORD_CREATE_H
#define CHWORD_CREATE_H

#include <set>
#include <map>
#include <tr1/unordered_map>
#include "edit_distance.h"
#include "index_create.h"

class CHWordCreator
{
public:
	void init(map<string, string>& config);
	bool create(tr1::unordered_map<string, int>& chword);
	bool rectify(const string& ori_word, vector<pair<string, int> >& rec_word);
	bool ifExist(const string& key);
	
	bool load();
	bool save();
private:
	int m_ed;
	static const int MAXWORDLEN = 8;
	IndexCreator idx_create;
};

#endif
