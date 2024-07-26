#include "bishun_create.h"

void BiShunCreator::init(const string& out_path, map<string, string>& config)
{
	string index_file = out_path;
	string invert_file = out_path;
	string index_id_file = out_path;
	string invert_id_file = out_path;
	map<string, string>::iterator ssIt;
	ssIt = config.find("edit_distance");
	m_ed = ssIt != config.end() ? atoi(ssIt->second.c_str()) : 2;
	ssIt = config.find("bishun_idx");
	index_file += ssIt != config.end() ? ssIt->second : "bishun.idx";
	ssIt = config.find("bishun_ivt");
	invert_file += ssIt != config.end() ? ssIt->second : "bishun.ivt";
	ssIt = config.find("bishun_idx_dict");
	index_id_file += ssIt != config.end() ? ssIt->second : "bishun.idx.dict";
	ssIt = config.find("bishun_ivt_dict");
	invert_id_file += ssIt != config.end() ? ssIt->second : "bishun.ivt.dict";
	idx_create.init(index_file, invert_file, index_id_file, invert_id_file);
}

bool BiShunCreator::create(map<string, string>& chword_bishun)
{
	int bishun_len, min_len;
	int index_id, invert_id;
	int weight = 100;
	char val[10] = {0};
	string bishun;
	map<string, string>::iterator ssIt;
	for(ssIt = chword_bishun.begin(); ssIt != chword_bishun.end(); ++ssIt)
	{
		bishun = ssIt->second;
		bishun_len = bishun.size();
		if(bishun_len >= MAXBISHUNLEN)
			continue;
		invert_id = idx_create.setInvertId(bishun);
		min_len = bishun_len / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		sprintf(val, "%d", bishun_len);
		for(int idx_len = min_len; idx_len <= 4; ++idx_len)
		{
			for(int sta = 0; bishun_len >= sta + idx_len; ++sta)
			{
				index_id = idx_create.setIndexId(bishun.substr(sta, idx_len) + val);
				idx_create.addInvertNode(index_id, invert_id, weight);
			}
		}
	}
	idx_create.sortInvertNode();
	return true;
}

bool BiShunCreator::rectify(const string& ori_bishun, vector<pair<string, int> >& rec_bishun)
{
	rec_bishun.clear();
	int bishun_len = ori_bishun.size(), min_len;
	if(bishun_len >= MAXBISHUNLEN)
		return false;
	int sta_len = bishun_len - m_ed, end_len = bishun_len + m_ed;
	end_len = end_len >= MAXBISHUNLEN ? MAXBISHUNLEN - 1 : end_len;
	if(sta_len <= 0)
		sta_len = 1;
	int index_id;
	char val[10] = {0};

	set<int> ivt_set;
	InvertNode* start = NULL;
	int cnt = 0, ed = 0;
	for(int i = sta_len; i <= end_len; ++i)
	{
		min_len = i / (m_ed + 1);
		if(min_len == 0)
			min_len = 1;
		sprintf(val, "%d", i);
		for(int sta = 0; bishun_len >= sta + min_len; ++sta)
		{
			index_id = idx_create.getIndexId(ori_bishun.substr(sta, min_len) + val);
			if(!idx_create.getIvtList(index_id, start, cnt))
				continue;
			for(int pos = 0; pos < cnt; ++start, ++pos)
			{
				if(ivt_set.find(start->invert_id) != ivt_set.end())
					continue;
				ivt_set.insert(start->invert_id);
				string bs = idx_create.getInvertById(start->invert_id);
				ed = edit_distance(ori_bishun, bs);
				if(ed > m_ed)
					continue;
				rec_bishun.push_back(make_pair(bs, start->weight));
			}
		}
	}
	return true;
}

bool BiShunCreator::load()
{
	if(!idx_create.load())
		return false;
	return true;
}

bool BiShunCreator::save()
{
	if(!idx_create.save())
		return false;
	return true;
}
