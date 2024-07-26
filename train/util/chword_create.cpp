#include "chword_create.h"

void CHWordCreator::init(const string& out_path, map<string, string>& config)
{
	string index_file = out_path;
	string invert_file = out_path;
	string index_id_file = out_path;
	string invert_id_file = out_path;
	map<string, string>::iterator ssIt;
	ssIt = config.find("edit_distance");
	m_ed = ssIt != config.end() ? atoi(ssIt->second.c_str()) : 2;
	ssIt = config.find("chword_idx");
	index_file += ssIt != config.end() ? ssIt->second : "chword.idx";
	ssIt = config.find("chword_ivt");
	invert_file += ssIt != config.end() ? ssIt->second : "chword.ivt";
	ssIt = config.find("chword_idx_dict");
	index_id_file += ssIt != config.end() ? ssIt->second : "chword.idx.dict";
	ssIt = config.find("chword_ivt_dict");
	invert_id_file += ssIt != config.end() ? ssIt->second : "chword.ivt.dict";
	idx_create.init(index_file, invert_file, index_id_file, invert_id_file);
}

bool CHWordCreator::create(tr1::unordered_map<string, int>& chword)
{
	int word_len, min_len;
	int index_id, invert_id;
	int weight;
	char val[10] = {0};
	string word;
	tr1::unordered_map<string, int>::iterator siIt;
	for(siIt = chword.begin(); siIt != chword.end(); ++siIt)
	{
		word = siIt->first;
		word_len = word.size() / 2;
		if(word_len >= MAXWORDLEN)
			continue;
		weight = siIt->second;
		invert_id = idx_create.setInvertId(word);
		min_len = word_len / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		sprintf(val, "%d", word_len);
		for(int idx_len = min_len; idx_len <= 4; ++idx_len)
		{
			for(int sta = 0; word_len >= sta + idx_len; ++sta)
			{
				index_id = idx_create.setIndexId(word.substr(sta*2, idx_len*2) + val);
				idx_create.addInvertNode(index_id, invert_id, weight);
			}
		}
	}
	idx_create.sortInvertNode();
	return true;
}

bool CHWordCreator::rectify(const string& ori_word, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	int word_len = ori_word.size(), min_len;
	if(word_len >= MAXWORDLEN)
		return false;
	int index_id, invert_id;
	char val[10] = {0};

	set<int> ivt_set;
	InvertNode* start = NULL;
	vector<Node> vec;
	int cnt = 0, ed = 0, maxWeight = 0;
	min_len = word_len / (m_ed + 1);
	if(min_len == 0)
		min_len = 1;
	sprintf(val, "%d", word_len);
	for(int sta = 0; word_len >= sta + min_len; ++sta)
	{
		index_id = idx_create.getIndexId(ori_word.substr(sta, min_len) + val);
		if(!idx_create.getIvtList(index_id, start, cnt))
			continue;
		for(int pos = 0; pos < cnt; ++start, ++pos)
		{
			if(ivt_set.find(start->invert_id) != ivt_set.end())
				continue;
			ivt_set.insert(start->invert_id);
			string curword = idx_create.getInvertById(start->invert_id);
			ed = edit_distance_ch(ori_word, curword);
			if(ed > m_ed)
				continue;
			if(maxWeight < start->weight)
				maxWeight = start->weight;
			vec.push_back(Node(start->invert_id, ed, start->weight));
		}
	}
	sort(vec.begin(), vec.end());
	for(int i = 0; i < vec.size() && i < 2; ++i)
	{
		string word = idx_create.getInvertById(vec[i].m_ivt_id);
		rec_word.push_back(make_pair(word, vec[i].m_weight * 100 / maxWeight));
	}
	return true;
}

bool CHWordCreator::load()
{
	if(!idx_create.load())
		return false;
	return true;
}

bool CHWordCreator::save()
{
	if(!idx_create.save())
		return false;
	return true;
}
