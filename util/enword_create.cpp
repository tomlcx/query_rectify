#include "enword_create.h"

void ENWordCreator::init(map<string, string>& config)
{
	map<string, string>::iterator ssIt;
	ssIt = config.find("out_path");
	string out_path = ssIt != config.end() ? ssIt->second : "out_path";
	string index_file = out_path;
	string invert_file = out_path;
	string index_id_file = out_path;
	string invert_id_file = out_path;
	ssIt = config.find("edit_distance");
	m_ed = ssIt != config.end() ? atoi(ssIt->second.c_str()) : 2;
	ssIt = config.find("enword_idx");
	index_file += ssIt != config.end() ? ssIt->second : "enword.idx";
	ssIt = config.find("enword_ivt");
	invert_file += ssIt != config.end() ? ssIt->second : "enword.ivt";
	ssIt = config.find("enword_idx_dict");
	index_id_file += ssIt != config.end() ? ssIt->second : "enword.idx.dict";
	ssIt = config.find("enword_ivt_dict");
	invert_id_file += ssIt != config.end() ? ssIt->second : "enword.ivt.dict";
	idx_create.init(index_file, invert_file, index_id_file, invert_id_file);
}

bool ENWordCreator::create(tr1::unordered_map<string, int>& enword)
{
	int word_len, min_len;
	int index_id, invert_id;
	int weight;
	char val[10] = {0};
	string word;
	tr1::unordered_map<string, int>::iterator siIt;
	for(siIt = enword.begin(); siIt != enword.end(); ++siIt)
	{
		word = siIt->first;
		word_len = word.size();
		if(word_len >= MAXWORDLEN || word_len <= 2)
			continue;
		weight = siIt->second;
		invert_id = idx_create.setInvertId(word);
		min_len = word_len / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		min_len = min_len > 4 ? 4 : min_len;
		sprintf(val, "%d", word_len);
		for(int idx_len = min_len; idx_len <= 4; ++idx_len)
		{
			for(int sta = 0; word_len >= sta + idx_len; ++sta)
			{
				index_id = idx_create.setIndexId(word.substr(sta, idx_len) + val);
				idx_create.addInvertNode(index_id, invert_id, weight);
			}
		}
	}
	idx_create.sortInvertNode();
	return true;
}

bool ENWordCreator::rectify(const string& ori_word, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	int word_len = ori_word.size(), min_len;
	if(word_len >= MAXWORDLEN || word_len <= 2)
		return false;
	int sta_len = word_len - m_ed, end_len = word_len + m_ed;
	end_len = end_len >= MAXWORDLEN ? MAXWORDLEN - 1 : end_len;
	if(sta_len <= 2)
		sta_len = 3;
	int index_id;
	char val[10] = {0};

	set<int> ivt_set;
	InvertNode* start = NULL;
	vector<Node> vec;
	int cnt = 0, ed = 0;

	char** d;
	for(int i = sta_len; i <= end_len; ++i)
	{
		min_len = i / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		min_len = min_len > 4 ? 4 : min_len;
		sprintf(val, "%d", i);

		if(word_len >= i)
			d = mallocForEditDistance(1, i);
		else
			d = mallocForEditDistance(1, word_len);
		
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
				//ed = edit_distance(ori_word, curword);
				if(word_len >= i)
					//ed = edit_distance(ori_word, curword, d);
					ed = edit_distance(const_cast<char*>(ori_word.c_str()), const_cast<char*>(curword.c_str()), d, m_ed);
				else
					ed = edit_distance(const_cast<char*>(curword.c_str()), const_cast<char*>(ori_word.c_str()), d, m_ed);
				
				if(ed > m_ed)
					continue;
				vec.push_back(Node(start->invert_id, ed, start->weight));
			}
		}
		freeForEditDistance(1, d);
	}
	sort(vec.begin(), vec.end());
	int pos = 0;
	for(int i = 0; i < vec.size() && (rec_word.size() < 10 || pos < 2); ++i)
	{
		string word = idx_create.getInvertById(vec[i].m_ivt_id);
		ed = vec[i].m_ed == 0 ? 0 : edit_distance_diff(ori_word, word);
		if(ed == 0)
		{
			if(pos == 2 && rec_word.size() > 3)++pos;
			rec_word.insert(rec_word.begin() + pos, make_pair(word, vec[i].m_weight));
			++pos;
		}
		else if(ed >= 2 && vec[i].m_ed >= 2 && ori_word.size() <= 5)
			continue;
		else
			rec_word.push_back(make_pair(word, vec[i].m_weight));
	}
	return true;
}

bool ENWordCreator::ifExist(const string& key)
{
	if(idx_create.getInvertId(key) == -1)
		return false;
	return true;
}

bool ENWordCreator::load()
{
	if(!idx_create.load())
		return false;
	return true;
}

bool ENWordCreator::save()
{
	if(!idx_create.save())
		return false;
	return true;
}
