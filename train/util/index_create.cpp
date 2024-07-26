#include "index_create.h"

static const int MAX_STR_LEN = 25;

void IndexCreator::init(string& index_file, string& invert_file, 
		string& index_id_file, string& invert_id_file)
{
	m_index_file = index_file;
	m_invert_file = invert_file;
	m_index_id_file = index_id_file;
	m_invert_id_file = invert_id_file;
}

int IndexCreator::getInvertId(const string& key)
{
	return m_hash_map_ivt[key];
}

int IndexCreator::getIndexId(const string& key)
{
	return m_hash_map_idx[key];
}

string IndexCreator::getIndexById(int index_id)
{
	if(index_id >= m_id_index.size())
		return "";
	return m_id_index[index_id];
}

string IndexCreator::getInvertById(int invert_id)
{
	if(invert_id >= m_id_invert.size())
		return "";
	return m_id_invert[invert_id];
}

int IndexCreator::setInvertId(const string& key)
{
	int invert_id;
	siIt = m_invert_id.find(key);
	if(siIt == m_invert_id.end())
	{
		invert_id = m_invert_id.size();
		m_invert_id.insert(make_pair(key, invert_id));
		m_id_invert.push_back(key);
	}
	else
		invert_id = siIt->second;
	return invert_id;
}

int IndexCreator::setIndexId(const string& key)
{
	int index_id;
	siIt = m_index_id.find(key);
	if(siIt == m_index_id.end())
	{
		index_id = m_index_id.size();
		m_index_id.insert(make_pair(key, index_id));
		m_id_index.push_back(key);
	}
	else
		index_id = siIt->second;
	return index_id;
}

bool IndexCreator::getIvtList(int index_id, InvertNode*& sta, int& cnt)
{
	if(index_id < 0 || index_id >= (int)m_idx.size()-1)
		return false;
	int pos = m_idx[index_id];
	sta = &m_ivt[pos];
	cnt = m_idx[index_id+1] - m_idx[index_id];
	return true;
}

bool IndexCreator::addInvertNode(int index_id, int invert_id, int weight)
{
	if(index_id < 0 || index_id > m_index.size())
		return false;
	InvertNode in(invert_id, weight);
	if(index_id == m_index.size())
	{
		vector<InvertNode> vin;
		m_index.push_back(vin);
	}
	m_index[index_id].push_back(in);
	return true;
}

void IndexCreator::sortInvertNode()
{
	for(int i = 0; i < m_index.size(); ++i)
		sort(m_index[i].begin(), m_index[i].end());
}

bool IndexCreator::load()
{
	FILE* idx_fp = fopen(m_index_file.c_str(), "rb");
	FILE* ivt_fp = fopen(m_invert_file.c_str(), "rb");
	if(!idx_fp || !ivt_fp)
		return false;
	int ivt_cnt, idx_cnt;
	fread(&idx_cnt, sizeof(int), 1, idx_fp);
	fread(&ivt_cnt, sizeof(int), 1, ivt_fp);
	m_idx.resize(idx_cnt);
	m_ivt.resize(ivt_cnt);
	int idx_num = fread(&m_idx[0], sizeof(int), idx_cnt, idx_fp);
	int ivt_num = fread(&m_ivt[0], sizeof(InvertNode), ivt_cnt, ivt_fp);
	if((idx_num != idx_cnt) || (ivt_num != ivt_cnt))
		return false;
	fclose(idx_fp);
	fclose(ivt_fp);

	m_hash_map_idx.load_serialized_hash_file(m_index_id_file.c_str(), -1);
	m_hash_map_ivt.load_serialized_hash_file(m_invert_id_file.c_str(), -1);

	if(!loadKey(m_id_index, m_index_id_file + ".reverse"))
		return false;
	if(!loadKey(m_id_invert, m_invert_id_file + ".reverse"))
		return false;
	return true;
}

bool IndexCreator::loadKey(vector<string>& m_id_key, string file)
{
	FILE* fp = fopen(file.c_str(), "rb");
	if(!fp)
		return false;
	int key_cnt;
	if(fread(&key_cnt, sizeof(int), 1, fp) != 1 || key_cnt <= 0)
		return false;
	char* buf = new char[key_cnt * MAX_STR_LEN];
	if(buf == NULL)
		return false;
	if(fread(&buf, sizeof(char), key_cnt * MAX_STR_LEN, fp) != key_cnt * MAX_STR_LEN)
		return false;
	m_id_key.resize(key_cnt);
	char tmp[50] = {0};
	for(int i = 0; i < key_cnt; ++i)
	{
		memcpy(tmp, buf + i * MAX_STR_LEN, MAX_STR_LEN);
		m_id_key[i] = tmp;
	}
	fclose(fp);
	return true;
}

bool IndexCreator::save()
{
	int idx_cnt = m_index.size() + 1, ivt_cnt = 0;
	FILE* idx_fp = fopen(m_index_file.c_str(), "wb");
	FILE* ivt_fp = fopen(m_invert_file.c_str(), "wb");
	if(!idx_fp || !ivt_fp)
		return false;
	fwrite(&idx_cnt, sizeof(int), 1, idx_fp);
	fseek(ivt_fp, sizeof(int), SEEK_SET);
	for(int i = 0; i < m_index.size(); ++i)
	{
		fwrite(&ivt_cnt, sizeof(int), 1, idx_fp);
		InvertNode* in_ptr = &m_index[i][0];
		for(int j = 1; j < m_index[i].size();)
		{
			while(j < m_index[i].size() && m_index[i][j].invert_id == in_ptr->invert_id)
			{
				in_ptr->weight += m_index[i][j].weight;
				++j;
			}
			if(j < m_index[i].size())
			{
				++ivt_cnt;
				fwrite(in_ptr, sizeof(InvertNode), 1, ivt_fp);
				in_ptr = &m_index[i][j];
			}
		}
		++ivt_cnt;
		fwrite(in_ptr, sizeof(InvertNode), 1, ivt_fp);
	}
	fwrite(&ivt_cnt, sizeof(int), 1, idx_fp);
	fseek(ivt_fp, 0, SEEK_SET);
	fwrite(&ivt_cnt, sizeof(int), 1, ivt_fp);
	fclose(idx_fp);
	fclose(ivt_fp);
	m_index.clear();

	int bucket = (int)(ceil(log((double)m_index_id.size())/log(2.0)));
	m_hash_map_idx.container_to_hash_file(m_index_id, bucket, m_index_id_file.c_str());
	m_index_id.clear();

	bucket = (int)(ceil(log((double)m_invert_id.size()) / log(2.0)));
	m_hash_map_ivt.container_to_hash_file(m_invert_id, bucket, m_invert_id_file.c_str());
	m_invert_id.clear();

	if(!saveKey(m_id_index, m_index_id_file + ".reverse"))
		return false;
	if(!saveKey(m_id_invert, m_invert_id_file + ".reverse"))
		return false;
	return true;
}

bool IndexCreator::saveKey(vector<string>& m_id_key, string file)
{
	int count = m_id_key.size();
	FILE* fp = fopen(file.c_str(), "wb");
	if(!fp)
		return false;
	fwrite(&count, sizeof(int), 1, fp);
	for(int i = 0; i < m_id_key.size(); ++i)
	{
		m_id_key[i].resize(MAX_STR_LEN);
		fwrite(m_id_key[i].data(), sizeof(char), MAX_STR_LEN, fp);
	}
	fclose(fp);
	m_id_key.clear();
	return true;
}

