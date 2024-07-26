#include "whole_pinyin_create.h"

bool PairCmp(const pair<int, int>& p1, const pair<int, int>& p2)
{
	return p1.first < p2.first || (p1.first == p2.first && p1.second > p2.second);
}

void WholePinYinCreator::init(map<string, string>& config)
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
	ssIt = config.find("whole_pinyin_idx");
	index_file += ssIt != config.end() ? ssIt->second : "whole_pinyin.idx";
	ssIt = config.find("whole_pinyin_ivt");
	invert_file += ssIt != config.end() ? ssIt->second : "whole_pinyin.ivt";
	ssIt = config.find("whole_pinyin_idx_dict");
	index_id_file += ssIt != config.end() ? ssIt->second : "whole_pinyin.idx.dict";
	ssIt = config.find("whole_pinyin_ivt_dict");
	invert_id_file += ssIt != config.end() ? ssIt->second : "whole_pinyin.ivt.dict";
	idx_create.init(index_file, invert_file, index_id_file, invert_id_file);
}

bool WholePinYinCreator::create(map<string, int>& wpy)
{
	int word_len, min_len, max_len;
	int index_id, invert_id;
	int weight;
	char val[10] = {0};
	map<string, int>::iterator siIt;
	for(siIt = wpy.begin(); siIt != wpy.end(); ++siIt)
	{
		const string& word = siIt->first;
		word_len = word.size();
		if(word_len >= MAXWORDLEN || word_len <= 2)
			continue;
		weight = siIt->second;
		invert_id = idx_create.setInvertId(word);
		min_len = word_len / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		max_len = min_len + 3;
		sprintf(val, "%d", word_len);
		for(int idx_len = min_len; idx_len <= max_len; ++idx_len)
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

bool WholePinYinCreator::rectify(string& ori_word, vector<pair<string, int> >& rec_word)
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

	vector<int> ivt_vec;
	set<int>::iterator ivtIt;
	InvertNode* start = NULL;
	vector<Node> vec;
	char** d;
	int byte_sum =0;
	int cnt = 0, ed = 0, mark = -1, times = 1, num = 0;
	for(int i = word_len; times / 2 <= m_ed; i = word_len + mark * times / 2)
	{
		mark *= -1;
		++times;
		if(i < sta_len || i > end_len)
			continue;
		min_len = i / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		sprintf(val, "%d", i);

		if(word_len >= i)
			d = mallocForEditDistance(1, i);
		else
			d = mallocForEditDistance(1, word_len);
		int ivt_num = idx_create.getMaxInvert();
		CBitMap bm(ivt_num + 10000);
		for(int sta = 0; word_len >= sta + min_len; ++sta)
		{
			index_id = idx_create.getIndexId(ori_word.substr(sta, min_len) + val);
			if(!idx_create.getIvtList(index_id, start, cnt))
				continue;
			bool find = false;
			for(int pos = 0; pos < cnt; ++start, ++pos)
			{
				if(bm.TestBit(start->invert_id))
					continue;
				bm.SetBit(start->invert_id);
				string curword = idx_create.getInvertById(start->invert_id);
				if(curword[0] != ori_word[0])
				{
					if(!find)
						continue;
					else
						break;
				}
				byte_sum += curword.size();
				//ed = edit_distance(ori_word, curword);
				if(word_len >= i)
					ed = edit_distance(const_cast<char*>(ori_word.c_str()), const_cast<char*>(curword.c_str()), d, m_ed);
				else
					ed = edit_distance(const_cast<char*>(curword.c_str()), const_cast<char*>(ori_word.c_str()), d, m_ed);
				if(ed > m_ed || (ed == m_ed && ori_word.size() < 6 && edit_distance_diff(ori_word, curword) != 0))
					continue;
				if(ed < m_ed)
					++num;
				vec.push_back(Node(start->invert_id, ed, start->weight));
				find = true;
			}
		}
		freeForEditDistance(1, d);
		if(byte_sum > 10000)
			break;
	}
	sort(vec.begin(), vec.end());
	for(int i = 0; i < vec.size() && i < 10; ++i)
	{
		string word = idx_create.getInvertById(vec[i].m_ivt_id);
		rec_word.push_back(make_pair(word, vec[i].m_weight));
	}
	return true;
}

bool WholePinYinCreator::load()
{
	if(!idx_create.load())
		return false;
	return true;
}

bool WholePinYinCreator::save()
{
	if(!idx_create.save())
		return false;
	return true;
}
