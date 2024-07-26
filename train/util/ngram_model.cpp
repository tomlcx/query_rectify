#include "ngram_model.h"

bool NgramModel::load(const string& pos_file, const string& prob_file)
{
	string file = m_out_path + pos_file;
	FILE* pos_fp = fopen(file.c_str(), "rb");
	file = m_out_path + prob_file;
	FILE* prob_fp = fopen(file.c_str(), "rb");
	if(!pos_fp || prob_fp)
		return false;

	int count = 0;
	if(fread(&count, sizeof(int), 1, pos_fp) != 1 || count == 0)
		return false;
	m_chword.resize(count);
	if(fread(&m_chword[0], sizeof(WordProb), count, pos_fp) != count)
		return false;
	fclose(pos_fp);

	count = 0;
	if(fread(&count, sizeof(int), 1, prob_fp) != 1 || count == 0)
		return false;
	m_rchword.resize(count);
	if(fread(&m_rchword[0], sizeof(ShiftProb), count, prob_fp) != count)
		return false;
	fclose(prob_fp);

	return true;
}

ushort NgramModel::getWordProb(ushort cur_code)
{
	return m_chword[cur_code].weight;
}

ushort NgramModel::getShiftProb(ushort cur_code, ushort rcode)
{
	int sta = m_chword[cur_code].start, end = m_chword[cur_code+1].start;
	int mid = (sta + end) / 2;
	while(sta < end)
	{
		if(m_rchword[mid].code == rcode)
			return m_rchword[mid].weight;
		else if(m_rchword[mid].code > rcode)
			end = mid;
		else
			sta = mid + 1;
		mid = (sta + end) / 2;
	}
	return 0xffff;
}

bool NgramModel::create(const string& fname)
{
	string file = m_in_path + fname;
	ifstream ifs(file.c_str());
	if(!ifs)
		return false;
	string line;
	vector<string> vec;
	int weight = 0;
	int sum = 0;
	ushort code, rcode;
	m_chword.resize(0x10000);
	map<ushort, map<ushort, int> > word_rword;
	map<ushort, int> word_rtotal;
	while(getline(ifs, line))
	{
		split(line, vec, "\t");
		if(vec.size() != 2 || vec[0].size() <= 2 || vec[0].size() % 2 != 0)
			continue;
		weight = atoi(vec[1].c_str());
		code = *(unsigned short *)&vec[0][0];
		for(int i = 0; i < vec[0].size()-2; i+=2)
		{
			sum += weight;
			rcode = *(unsigned short *)&vec[0][i+2];
			if(word_rword[code].find(rcode) == word_rword[code].end())
			{
				m_chword[code].start += 1;
			}
			word_rword[code][rcode] += weight;
			word_rtotal[code] += weight;
			code = rcode;
		}
	}
	ifs.close();

	for(int i = 1; i < 0x10000; ++i)
		m_chword[i].start += m_chword[i-1].start;
	m_rchword.resize(m_chword[0xffff].start);
	int pos = 0;
	ushort log_left_total;
	map<ushort, map<ushort, int> >::iterator wIt;
char ch[3]={0};
	for(wIt = word_rword.begin(); wIt != word_rword.end(); ++wIt)
	{
		map<ushort, int>& rword = wIt->second;
		log_left_total = (ushort)(log(word_rtotal[wIt->first]) * 100);
memcpy(ch, &wIt->first, 2);
cout<<ch<<endl;
cout<<"log_left_total: "<<log_left_total<<endl;
		for(map<ushort, int>::iterator rIt = rword.begin(); rIt != rword.end(); ++rIt)
		{
			m_rchword[pos].code = rIt->first;
			m_rchword[pos].weight = log_left_total - (ushort)(log(rIt->second)*100);
memcpy(ch, &rIt->first, 2);
cout<<"rword:"<<ch<<"r weight: "<<m_rchword[pos].weight<<endl;
			++pos;
			--(m_chword[wIt->first].start);
		}
		m_chword[wIt->first].weight = (ushort)(log(sum) * 100) - log_left_total;
cout<<"weight: "<<m_chword[wIt->first].weight<<"sum:"<<sum<<"log sum:"<<(ushort)(log(sum) * 100)<<endl;
	}
cout<<"log(10):"<<log(10)<<"log(8):"<<(int)(log(8)*100)<<endl;
	return true;
}

void NgramModel::findBestPath(vector<vector<ushort> >& words, vector<pair<string, int> >& rec_word)
{
	int i, pos;
	ushort cur_code, rcode, prob, min;
	vector<vector<ushort> > probMatrix = words;
	vector<vector<ushort> > pathMatrix = words;
	for(i = 0; i < words[0].size(); ++i)
		probMatrix[0][i] = getWordProb(words[0][i]);
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
	for(i = 0; i < 2 && i < words[words.size()-1].size(); ++i)
	{
		pos = getMinValPos(probMatrix[words.size()-1], prob);
		if(pos == -1)
			break;
		string str;
		vector<ushort> rs;
		for(int idx = words.size()-1; idx >= 0; --idx)
		{
			rs.push_back(words[idx][pos]);
			pos = pathMatrix[idx][pos];
		}
		for(int j = 0; j < rs.size(); ++j)
		{
			memcpy(word, &rs[j], sizeof(ushort));
			str += word;
		}
		int weight = prob == 0 ? 100 : words.size() * 800 / prob;
		weight = weight > 100 ? 100 : weight;
		rec_word.push_back(make_pair(str,  weight));
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

bool NgramModel::save(const string& pos_file, const string& prob_file)
{
	string file = m_out_path + pos_file;
	FILE*  pos_fp = fopen(file.c_str(), "wb");
	file = m_out_path + prob_file;
	FILE*  prob_fp = fopen(file.c_str(), "wb");
	if(!pos_fp || !prob_fp)
		return false;
	int count = m_chword.size();
	fwrite(&count, sizeof(int), 1, pos_fp);
	fwrite(&m_chword[0], sizeof(WordProb), count, pos_fp);
	fclose(pos_fp);
	count = m_rchword.size();
	fwrite(&count, sizeof(int), 1, prob_fp);
	fwrite(&m_rchword[0], sizeof(ShiftProb), count, prob_fp);
	fclose(prob_fp);
	return true;
}

