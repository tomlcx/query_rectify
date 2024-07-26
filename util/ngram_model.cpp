#include "ngram_model.h"

void NgramModel::init(map<string, string>& config)
{
	map<string, string>::iterator ssIt;
	ssIt = config.find("out_path");
	m_out_path = ssIt != config.end() ? ssIt->second : "out_path";
	ssIt = config.find("ngram_idx");
	m_pos_file = ssIt != config.end() ? ssIt->second : "ngram_idx";
	ssIt = config.find("ngram_prob");
	m_prob_file = ssIt != config.end() ? ssIt->second : "ngram_prob";
}

bool NgramModel::load()
{
	string file = m_out_path + m_pos_file;
	FILE* pos_fp = fopen(file.c_str(), "rb");
	file = m_out_path + m_prob_file;
	FILE* prob_fp = fopen(file.c_str(), "rb");
	if(!pos_fp || !prob_fp)
		return false;

	int count = 0;
	if(fread(&count, sizeof(int), 1, pos_fp) != 1 || count == 0)
		return false;
	m_word.resize(count);
	if(fread(&m_word[0], sizeof(WordProb), count, pos_fp) != count)
		return false;
	fclose(pos_fp);

	count = 0;
	if(fread(&count, sizeof(int), 1, prob_fp) != 1 || count == 0)
		return false;
	m_rword.resize(count);
	if(fread(&m_rword[0], sizeof(ShiftProb), count, prob_fp) != count)
		return false;
	fclose(prob_fp);

	return true;
}

ushort NgramModel::getWordProb(ushort cur_code)
{
	return m_word[cur_code].weight;
}

ushort NgramModel::getShiftProb(ushort cur_code, ushort rcode)
{
	int sta = m_word[cur_code].start, end = m_word[cur_code+1].start;
	int mid = (sta + end) / 2;
	while(sta < end)
	{
		if(m_rword[mid].code == rcode)
			return m_rword[mid].weight;
		else if(m_rword[mid].code > rcode)
			end = mid;
		else
			sta = mid + 1;
		mid = (sta + end) / 2;
	}
	return 0xffff;
}

bool NgramModel::create(map<string, int>& words2weight)
{
	int weight = 0;
	long long sum = 0;
	ushort code, rcode;
	m_word.resize(0x10000);
	map<ushort, map<ushort, int> > word_rword;
	map<ushort, int> word_rtotal;
	map<string, int>::iterator it;
	for(it = words2weight.begin(); it != words2weight.end(); ++it)
	{
		const string& words = it->first;
		if(words.size() <= 2)
			continue;
		weight = it->second;
		code = *(unsigned short *)&words[0];
		for(int i = 0; i < words.size()-2; i += 2)
		{
			sum += weight;
			rcode = *(unsigned short *)&words[i+2];
			if(word_rword[code].find(rcode) == word_rword[code].end())
				m_word[code].start += 1;
			word_rword[code][rcode] += weight;
			word_rtotal[code] += weight;
			code = rcode;
		}
	}

	map<ushort, map<ushort, int> >::iterator wIt;
	map<ushort, int>::iterator rIt, tmpIt;
	for(wIt = word_rword.begin(); wIt != word_rword.end(); ++wIt)
	{
		map<ushort, int>& rword = wIt->second;
		for(rIt = rword.begin(); rIt != rword.end();)
		{
			if(rIt->second < 90)
			{
				tmpIt = rIt;
				++tmpIt;
				rword.erase(rIt);
				rIt = tmpIt;
				--(m_word[wIt->first].start);
			}
			else
				++rIt;
		}
	}
	for(int i = 1; i < 0x10000; ++i)
		m_word[i].start += m_word[i-1].start;
	m_rword.resize(m_word[0xffff].start);
	int pos = 0;
	ushort log_left_total;
	for(wIt = word_rword.begin(); wIt != word_rword.end(); ++wIt)
	{
		map<ushort, int>& rword = wIt->second;
		log_left_total = (ushort)(log((double)word_rtotal[wIt->first]) * 100);
		m_word[wIt->first].weight = (ushort)(log((long double)sum) * 100) - log_left_total;
		for(map<ushort, int>::iterator rIt = rword.begin(); rIt != rword.end(); ++rIt)
		{
			m_rword[pos].code = rIt->first;
			m_rword[pos].weight = log_left_total - (ushort)(log((double)rIt->second)*100);
			++pos;
			--(m_word[wIt->first].start);
		}
	}
	return true;
}

void NgramModel::findBestPath(vector<vector<ushort> >& words, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	double cur_weight;
	int i, pos;
	ushort cur_code, rcode, prob, min;
	vector<vector<ushort> > probMatrix = words;
	vector<vector<ushort> > pathMatrix = words;
	for(i = 0; i < words[0].size(); ++i)
		probMatrix[0][i] = 0;
	for(i = 1; i < words.size(); ++i)
	{
		for(int j = 0; j < words[i].size(); ++j)
		{
			pos = 0;
			min = 0xffff;
			rcode = words[i][j];
			for(int pre = 0; pre < words[i-1].size(); ++pre)
			{
				cur_code = words[i-1][pre];
				prob = getShiftProb(cur_code, rcode);
				if(prob == 0xffff || probMatrix[i-1][pre] == 0xffff)
					continue;
				if(min > prob + probMatrix[i-1][pre])
				{
					min = prob + probMatrix[i-1][pre];
					pos = pre;
				}
			}
			probMatrix[i][j] = min;
			pathMatrix[i][j] = pos;
		}
	}
	char word[3] = {0};
	for(i = 0; i < 1 && i < words[words.size()-1].size(); ++i)
	{
		pos = getMinValPos(probMatrix[words.size()-1], prob);
		if(pos == -1)
			break;
		probMatrix[words.size()-2][pathMatrix[words.size()-1][pos]] = 0xffff;
		int weight = prob == 0 ? 100 : (2000 + 30000 * (words.size()-2)) / prob;
		if(weight < 40)
			break;
		string str;
		vector<ushort> rs;
		for(int idx = words.size()-1; idx >= 0; --idx)
		{
			rs.push_back(words[idx][pos]);
			pos = pathMatrix[idx][pos];
		}
		int j = rs.size() - 2;
		if(rs[j] == *(ushort*)tag[NUM] || rs[j] == *(ushort*)tag[ALP])
			--j;
		for(; j >= 1; --j)
		{
			memcpy(word, &rs[j], sizeof(ushort));
			str += word;
		}
		rec_word.push_back(make_pair(str,  prob));
	}
	if(i == 0)
		return;

	for(i = 0; i < 1 && i < words[words.size()-2].size(); ++i)
	{
		pos = getMinValPos(probMatrix[words.size()-2], prob);
		if(pos == -1)
			break;
		prob += 400;
		int weight = prob == 0 ? 100 : (2000 + 30000 * (words.size()-2)) / prob;
		if(weight < 40)
			break;
		string str;
		vector<ushort> rs;
		for(int idx = words.size()-2; idx >= 0; --idx)
		{
			rs.push_back(words[idx][pos]);
			pos = pathMatrix[idx][pos];
		}
		int j = rs.size() - 2;
		if(rs[j] == *(ushort*)tag[NUM] || rs[j] == *(ushort*)tag[ALP])
			--j;
		for(; j >= 0; --j)
		{
			memcpy(word, &rs[j], sizeof(ushort));
			str += word;
		}
		rec_word.push_back(make_pair(str,  prob));
	}
}

int NgramModel::getMinValPos(vector<ushort>& vec, ushort& prob)
{
	int pos;
	ushort min = 0xffff;
	for(int i = 0; i < vec.size(); ++i)
	{
		if(min > vec[i])
		{
			min = vec[i];
			pos = i;
		}
	}
	prob = min;
	if(min == 0xffff)
		return -1;
	vec[pos] = 0xffff;
	return pos;
}

bool NgramModel::save()
{
	string file = m_out_path + m_pos_file;
	FILE*  pos_fp = fopen(file.c_str(), "wb");
	file = m_out_path + m_prob_file;
	FILE*  prob_fp = fopen(file.c_str(), "wb");
	if(!pos_fp || !prob_fp)
		return false;
	int count = m_word.size();
	fwrite(&count, sizeof(int), 1, pos_fp);
	fwrite(&m_word[0], sizeof(WordProb), count, pos_fp);
	fclose(pos_fp);
	count = m_rword.size();
	fwrite(&count, sizeof(int), 1, prob_fp);
	fwrite(&m_rword[0], sizeof(ShiftProb), count, prob_fp);
	fclose(prob_fp);
	return true;
}

