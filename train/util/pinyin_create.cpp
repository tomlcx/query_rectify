#include "pinyin_create.h"

void PinYinCreator::init(const string& out_path, map<string, string>& config)
{
	string index_file = out_path;
	string invert_file = out_path;
	string index_id_file = out_path;
	string invert_id_file = out_path;
	map<string, string>::iterator ssIt;
	ssIt = config.find("edit_distance");
	m_ed = ssIt != config.end() ? atoi(ssIt->second.c_str()) : 2;
	ssIt = config.find("first_pinyin_idx");
	index_file += ssIt != config.end() ? ssIt->second : "first_pinyin.idx";
	ssIt = config.find("first_pinyin_ivt");
	invert_file += ssIt != config.end() ? ssIt->second : "first_pinyin.ivt";
	ssIt = config.find("first_pinyin_idx_dict");
	index_id_file += ssIt != config.end() ? ssIt->second : "first_pinyin.idx.dict";
	ssIt = config.find("first_pinyin_ivt_dict");
	invert_id_file += ssIt != config.end() ? ssIt->second : "first_pinyin.ivt.dict";
	idx_create.init(index_file, invert_file, index_id_file, invert_id_file);
}

bool PinYinCreator::create(tr1::unordered_map<string, int>& chword, map<string, set<string> >& chword_pinyin)
{
	int word_len, min_len;
	int index_id, invert_id;
	int weight;
	char val[10] = {0};
	string word;
	vector<string> first_pinyin;
	tr1::unordered_map<string, int>::iterator siIt;
	for(siIt = chword.begin(); siIt != chword.end(); ++siIt)
	{
		word = siIt->first;
		weight = siIt->second;
		word_len = word.size() / 2;
		if(word_len >= MAXWORDLEN)
			continue;
		if(!getFirstPinYin(word, chword_pinyin, first_pinyin))
			continue;
		sort(first_pinyin.begin(), first_pinyin.end());
		first_pinyin.erase(unique(first_pinyin.begin(), first_pinyin.end()), first_pinyin.end());
		invert_id = idx_create.setInvertId(word);
		for(int i = 0; i < first_pinyin.size(); ++i)
		{
			index_id = idx_create.setIndexId(first_pinyin[i]);
			idx_create.addInvertNode(index_id, invert_id, weight);
		}
	}
	idx_create.sortInvertNode();
	return true;
}

bool PinYinCreator::rectify(const string& fpy, const string& ori_word, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	int py_len = fpy.size();
	int index_id, invert_id;
	char val[10] = {0};

	set<int> ivt_set;
	InvertNode* start = NULL;
	vector<Node> vec;
	int cnt = 0, ed = 0, maxWeight = 0;
	index_id = idx_create.getIndexId(fpy);
	if(!idx_create.getIvtList(index_id, start, cnt))
		return false;
	for(int pos = 0; pos < cnt; ++start, ++pos)
	{
		if(ivt_set.find(start->invert_id) != ivt_set.end())
			continue;
		ivt_set.insert(start->invert_id);
		string word = idx_create.getInvertById(start->invert_id);
		ed = edit_distance_fpy(ori_word, word);
		if(ed > m_ed)
			continue;
		if(maxWeight < start->weight)
			maxWeight = start->weight;
		vec.push_back(Node(start->invert_id, ed, start->weight));
	}
	sort(vec.begin(), vec.end());
	for(int i = 0; i < vec.size() && i < 2; ++i)
	{
		string word = idx_create.getInvertById(vec[i].m_ivt_id);
		rec_word.push_back(make_pair(word, vec[i].m_weight * 100 / maxWeight));
	}
	return true;
}

bool PinYinCreator::load()
{
	if(!idx_create.load())
		return false;
	return true;
}

bool PinYinCreator::save()
{
	if(!idx_create.save())
		return false;
	return true;
}

bool PinYinCreator::getPinYin(const string& chword, map<string, set<string> >& chword_pinyin, 
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

bool PinYinCreator::getFirstPinYin(const string& chword, map<string, set<string> >& chword_pinyin, 
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

bool PinYinCreator::getPinYin(const string& chword, int pos, vector<string>& vec, string str, 
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

bool PinYinCreator::getFirstPinYin(const string& chword, int pos, string str, map<string, set<string> >& chword_pinyin, vector<string>& first_pinyin)
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

