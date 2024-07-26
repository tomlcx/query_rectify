#ifndef ENWORD_CREATE_H
#define ENWORD_CREATE_H

#include <iostream>
#include <set>
#include <map>
#include <tr1/unordered_map>
#include "edit_distance.h"
#include "index_create.h"

class ENWordCreator
{
public:
	void init(map<string, string>& config);
	bool load();
	bool create(tr1::unordered_map<string, int>& enword);
	bool rectify(const string& ori_word, vector<pair<string, int> >& rec_word);
	bool ifExist(const string& key);
	bool save();
private:
	int m_ed;
	static const int MAXWORDLEN = 13;
	IndexCreator idx_create;
};

#endif
