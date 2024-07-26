#ifndef ENWORD_CREATE_H
#define ENWORD_CREATE_H

#include <set>
#include <map>
#include <tr1/unordered_map>
#include "edit_distance.h"
#include "index_create.h"

class ENWordCreator
{
public:
	void init(const string& out_path, map<string, string>& config);
	bool load();
	bool create(tr1::unordered_map<string, int>& enword);
	bool rectify(const string& ori_word, vector<pair<string, int> >& rec_word);
	bool save();
private:
	int m_ed;
	static const int MAXWORDLEN = 13;
	IndexCreator idx_create;
};

#endif
