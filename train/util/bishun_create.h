#ifndef BISHUN_CREATE_H
#define BISHUN_CREATE_H

#include <set>
#include <map>
#include "edit_distance.h"
#include "index_create.h"

class BiShunCreator
{
public:
	void init(const string& out_path, map<string, string>& config);
	bool load();
	bool create(map<string, string>& chword_bishun);
	bool rectify(const string& ori_bishun, vector<pair<string, int> >& rec_bishun);
	bool save();
private:
	int m_ed;
	static const int MAXBISHUNLEN = 17;
	IndexCreator idx_create;
};

#endif
