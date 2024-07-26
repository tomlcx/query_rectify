#ifndef INDEX_CREATE_H
#define INDEX_CREATE_H

#include <math.h>
#include <vector>
#include <string>
#include <ext/hash_map>
#include "hash_wrap.h"
#include "nokey_static_hash.h"
using namespace std;
using namespace __gnu_cxx;

struct InvertNode
{
	InvertNode(int id = 0, int wt = 0):invert_id(id), weight(wt)
	{
	}
	int invert_id;
	int weight;
	bool operator >(const InvertNode& in) const
	{
		return (invert_id > in.invert_id) ||
			(invert_id == in.invert_id && weight > in.weight);
	}
	bool operator <(const InvertNode& in) const
	{
		return (invert_id < in.invert_id) ||
			(invert_id == in.invert_id && weight < in.weight);
	}
	bool operator ==(const InvertNode& in) const
	{
		return invert_id == in.invert_id;
	}
};

class IndexCreator
{
public:
	void init(string& index_file, string& invert_file,
		string& index_id_file, string& invert_id_file);
	bool load();
	bool loadKey(vector<string>& m_id_key, string file);

	int getInvertId(const string& key);
	int getIndexId(const string& key);
	int setInvertId(const string& key);
	int setIndexId(const string& key);

	string getIndexById(int index_id);
	string getInvertById(int invert_id);

	bool getIvtList(int index_id, InvertNode*& sta, int& cnt);
	int getMaxInvert();

	bool addInvertNode(int index_id, int invert_id, int weight);
	void sortInvertNode();
	bool save();
	bool saveKey(vector<string>& m_id_key, string file);

private:
	vector<vector<InvertNode> > m_index;
	hash_map<string, int> m_invert_id;
	vector<string> m_id_invert;
	hash_map<string, int> m_index_id;
	vector<string> m_id_index;

	hash_map<string, int>::iterator siIt;
private:
	int m_max_ivt;
	vector<int> m_idx;
	vector<InvertNode> m_ivt;
private:
	string m_index_file, m_invert_file;
	string m_index_id_file, m_invert_id_file;
	static_hash_map<string, int> m_hash_map_idx;
	static_hash_map<string, int> m_hash_map_ivt;
};

#endif
