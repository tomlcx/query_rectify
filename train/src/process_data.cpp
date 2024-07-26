#include "process_data.h"

bool ProcessData::processData(string& infile, bool seg, string seperator, int def)
{
	string file = m_in_path + infile;
	ifstream ifs(file.c_str());
	if(!ifs)
		return false;
	string line;
	int weight, code;
	m_sin_chword.resize(0x10000);
	vector<string> vecWord, tmp, vec;
	vector<CHARTYPE> vecMark;

	while(getline(ifs, line))
	{
		trim(line);
		if(line == "")
			continue;
		split(line, tmp, seperator);
		if(tmp[0] == "")
			continue;
		if(tmp.size() < 2 || (weight = atoi(tmp[1].c_str())) == 0)
			weight = def;
		to_lower(tmp[0]);
		m_cc.Convert_t2s(const_cast<char*>(tmp[0].c_str()));
		splitCHAlNum(tmp[0], vecWord, vecMark, true);
		for(int i = 0; i < vecMark.size(); ++i)
		{
			if(vecMark[i] == ALPHA)
			{
				m_enword[vecWord[i]] += weight;
				if(i == 0 && vecWord[i].size() < 15 && i + 1 < vecMark.size())
				{
					if(vecMark[i+1] == NUMBER || (vecMark[i+1] == CHWORD && vecWord[i+1].size() <= 8))
						m_word_rword_weight[vecWord[i]][vecWord[i+1]] += weight;
				}
				else if(i > 0 && vecMark[i-1] == NUMBER)
					m_word_rword_weight[vecWord[i-1]][vecWord[i]] += weight;
			}
			else if(vecMark[i] == CHWORD)
			{
				for(int pos = 0; pos < vecWord[i].size(); pos += 2)
				{
					code = *(unsigned short*)&vecWord[i][pos];
					m_sin_chword[code] += weight;
				}
				string tag_str;
				if(i == 0 || vecMark[i-1] == ENPUNC || vecMark[i-1] == CHPUNC)
				{
					if(seg)
					{
						/*m_probSeg.maxSegment(vecWord[i], vec);
						for(int pos = 0; pos < vec.size(); ++pos)
						{
							if(vec[pos].size() >= 4)
							{
								m_chword[vec[pos]] += weight;
								m_whole_pinyin[vec[pos]] += weight;
							}
						}*/
					}
					else if(vecWord[i].size() >= 4)
					{
						m_chword[vecWord[i]] += weight;
						m_whole_pinyin[vecWord[i]] += weight;
					}

					if(vecWord[i].size() >= 4)
						tag_str = tag[BEG] + vecWord[i] + tag[END];
				}
				else
				{
					tag_str = tag[BEG];
					if(vecMark[i-1] == NUMBER)
						tag_str += tag[NUM];
					else if(vecMark[i-1] == ALPHA)
						tag_str += tag[ALP];
					tag_str += vecWord[i] + tag[END];
				}
				if(!tag_str.empty())
					words2weight[tag_str] += weight;
				if(i == 0)
				{
					int j;
					string tmp;
					for(j = i + 1; j < vecMark.size(); ++j)
					{
						if(j == i + 1)
						{
							if(vecMark[j] != ALPHA)
								break;
							tmp += vecWord[j];
							m_whole_pinyin[vecWord[i] + tmp] += weight;
						}
						else if(vecMark[j] == NUMBER)
						{
							tmp += vecWord[j];
						}
						else
						{
							break;
						}
					}
					if(!tmp.empty())
					{
						if(vecWord[i].size() <= 10 && tmp.size() <= 8)
						{
							m_word_rword_weight[vecWord[i]][tmp] += weight;
							m_word_rword_weight[tmp][vecWord[i]] += weight;
						}
					}
				}
			}
		}
	}
	ifs.close();
	return true;
}

void ProcessData::sortCHENWord()
{
	int i;
	multimap<int, string, greater<int> > m_mmap_word;
	multimap<int, string, greater<int> >::iterator misIt;
	for(siIt = m_chword.begin(); siIt != m_chword.end(); ++siIt)
	{
		for(i = 0; i < m_filt.size(); ++i)
		{
			if(siIt->first.find(m_filt[i]) == 0)
				break;
		}
		if(i != m_filt.size())
			continue;
		m_mmap_word.insert(make_pair(siIt->second, siIt->first));
	}
	m_chword.clear();
	int count = 0;
	ofstream ofs_ch("ch.txt");
	for(misIt = m_mmap_word.begin(); misIt != m_mmap_word.end(); ++misIt)
	{
		if(misIt->second.size() <= 6 && misIt->second.size() >= 4)
			ofs_ch << misIt->second << '\t' << misIt->first << endl;
		if(!(m_ch_max_num != 0 && count > m_ch_max_num) &&
			!(m_ch_thresh != 0 && misIt->first < m_ch_thresh))
		{
			++count;
			m_chword.insert(make_pair(misIt->second, misIt->first));
		}
		else
			break;
	}
	ofs_ch.close();
	m_mmap_word.clear();
	cout << "chword num: " << count << endl;

	vector<CHARTYPE> vecMark;
	vector<string> vecWord;
	vector<string> pinyin;
	for(siIt1 = m_whole_pinyin.begin(); siIt1 != m_whole_pinyin.end(); ++siIt1)
		m_mmap_word.insert(make_pair(siIt1->second, siIt1->first));
	m_whole_pinyin.clear();
	count = 0;
	for(misIt = m_mmap_word.begin(); misIt != m_mmap_word.end(); ++misIt)
	{
		if(!(m_wpy_max_num != 0 && count > m_wpy_max_num) &&
			!(m_wpy_thresh != 0 && misIt->first < m_wpy_thresh))
		{
			string& word = misIt->second;
			splitCHEN(word, vecWord, vecMark);
			if(vecMark.size() > 2 || vecMark[0] != CHWORD || word.size()+1 >= STRMAXNUM)
				continue;
			for(i = 0; i < m_filt.size(); ++i)
			{
				if(vecWord[0].find(m_filt[i]) == 0)
					break;
			}
			if(i != m_filt.size())
				continue;
			if(!getWholePinYin(vecWord[0], m_chword_pinyin, pinyin))
				continue;
			string en = vecWord.size() == 2 ? vecWord[1] : "";
			for(int i = 0; i < pinyin.size(); ++i)
			{
				m_wpy2word[pinyin[i] + en].push_back(make_pair(word, misIt->first));
				m_whole_pinyin[pinyin[i] + en] += misIt->first;
			}
			getFirstPinYin(vecWord[0], m_chword_pinyin, pinyin);
			sort(pinyin.begin(), pinyin.end());
			pinyin.erase(unique(pinyin.begin(), pinyin.end()), pinyin.end());
			for(int i = 0; i < pinyin.size(); ++i)
				m_fpy2word[pinyin[i] + en].push_back(make_pair(word, misIt->first));
			++count;
		}
		else
			break;
	}
	m_mmap_word.clear();
	cout << "whole pinyin num: " << count << endl;

	for(siIt = m_enword.begin(); siIt != m_enword.end(); ++siIt)
	{
		for(i = 0; i < m_filt.size(); ++i)
		{
			if(siIt->first.find(m_filt[i]) == 0)
				break;
		}
		if(i != m_filt.size())
			continue;
		m_mmap_word.insert(make_pair(siIt->second, siIt->first));
	}
	m_enword.clear();
	count = 0;
	ofstream ofs_en("en.txt");
	for(misIt = m_mmap_word.begin(); misIt != m_mmap_word.end(); ++misIt)
	{
		if(misIt->second.size() > 1)
			ofs_en << misIt->second << '\t' << misIt->first << endl;
		if(!(m_en_max_num != 0 && count > m_en_max_num) &&
			!(m_en_thresh != 0 && misIt->first < m_en_thresh))
		{
			if(m_whole_pinyin.find(misIt->second) != m_whole_pinyin.end() && misIt->first < m_en_thresh * 40)
			{
				cout << "en:" << misIt->second << endl;
				continue;
			}
			++count;
			m_enword.insert(make_pair(misIt->second, misIt->first));
		}
		else
			break;
	}
	ofs_en.close();
	m_mmap_word.clear();
	cout << "enword num: " << count << endl;
}

bool ProcessData::createIndex()
{
	cout << "create ngram model start..." << endl;
	if(!m_nm.create(words2weight) || !m_nm.save())
		return false;
	cout << "create ngram model end" << endl;
	
	cout << "create whole pinyin index start..." << endl;
	if(!m_whole_pinyin_create.create(m_whole_pinyin) || !m_whole_pinyin_create.save())
		return false;
	cout << "create whole pinyin index end" << endl;

	cout << "create chword index start..." << endl;
	if(!m_chword_create.create(m_chword) || !m_chword_create.save())
		return false;
	cout << "create chword index end" << endl;

	cout << "create enword index start..." << endl;
	if(!m_enword_create.create(m_enword) || !m_enword_create.save())
		return false;
	cout << "create enword index end" << endl;
	return true;
}

bool ProcessData::readPinYin(const string& fname)
{
	string file = m_in_path + fname;
	ifstream ifs(file.c_str());
	if(!ifs)
		return false;
	string line, word;
	vector<string> tmp;
	while(getline(ifs, line))
	{
		trim(line);
		split(line, tmp, "\t");
		if(tmp.size() != 2 || tmp[0] == "" || tmp[1] == "")
			continue;
		tmp[0].erase(tmp[0].size()-1);
		for(int i = 0; i < tmp[1].size(); i += 2)
		{
			word = tmp[1].substr(i, 2);
			m_pinyin_chword[tmp[0]].insert(word);
			m_chword_pinyin[word].insert(tmp[0]);
		}
	}
	return true;
}

bool ProcessData::readFiltSet(const string& fname)
{
	string file = m_in_path + fname;
	ifstream ifs(file.c_str());
	if(!ifs)
		return false;
	string line;
	while(getline(ifs, line))
	{
		trim(line);
		m_filt.push_back(line);
	}
	return true;
}

void ProcessData::filtCHWord()
{
	int wordId;
	set<string>::iterator setIt, tmpIt;
	for(sSIt = m_pinyin_chword.begin(); sSIt != m_pinyin_chword.end(); ++sSIt)
	{
		set<string>& words = sSIt->second;
		for(setIt = words.begin(); setIt != words.end();)
		{
			wordId = *(unsigned short*)&(*setIt)[0];
			tmpIt = setIt;
			++tmpIt;
			if(wordId >= m_sin_chword.size() || m_sin_chword[wordId] <= 10)
				words.erase(setIt);
			setIt = tmpIt;
		}
	}
}

bool ProcessData::savePinYin(const string& fname)
{
	string file = m_out_path + fname;
	ofstream ofs(file.c_str());
	file += ".reverse";
	ofstream ofs_rvs(file.c_str());
	if(!ofs || !ofs_rvs)
		return false;
	set<string>::iterator setIt;
	for(sSIt = m_pinyin_chword.begin(); sSIt != m_pinyin_chword.end(); ++sSIt)
	{
		if(sSIt->second.size() == 0)
			continue;
		ofs << sSIt->first;
		set<string>& words = sSIt->second;
		for(setIt = words.begin(); setIt != words.end(); ++setIt)
			ofs << "\t" << *setIt;
		ofs << endl;
	}
	m_pinyin_chword.clear();

	for(sSIt = m_chword_pinyin.begin(); sSIt != m_chword_pinyin.end(); ++sSIt)
	{
		if(sSIt->second.size() == 0)
			continue;
		ofs_rvs << sSIt->first;
		set<string>& pinyin = sSIt->second;
		for(setIt = pinyin.begin(); setIt != pinyin.end(); ++setIt)
			ofs_rvs << "\t" << *setIt;
		ofs_rvs << endl;
	}
	m_chword_pinyin.clear();
	ofs.close();
	ofs_rvs.close();
	return true;
}

bool ProcessData::savePinYinToWord(const string& fname)
{
	string file = m_out_path + "whole_" + fname;
	if(!m_wpy.write(m_wpy2word, file))
	{
		cout << "whole pinyin word data write fail!" << endl;
		return false;
	}

	file = m_out_path + "first_" + fname;
	if(!m_fpy.write(m_fpy2word, file))
	{
		cout << "first pinyin word data write fail!" << endl;
		return false;
	}
}

bool ProcessData::saveWordCooccur(const string& fname)
{
	string file = m_out_path + fname;
	ofstream ofs((file + ".txt").c_str());
	if(!ofs)
		return false;
	hash_map<string, vector<string> > hmap;
	multimap<int, string, greater<int> > mmap;
	map<string, map<string, int> >::iterator it;
	for(it = m_word_rword_weight.begin(); it != m_word_rword_weight.end(); ++it)
	{
		mmap.clear();
		map<string, int>& si = it->second;
		for(map<string, int>::iterator siIt = si.begin(); siIt != si.end(); ++siIt)
		{
			if(m_word_cooccur_num > siIt->second || siIt->first.size()+1 >= STRMAXNUM)
				continue;
			mmap.insert(make_pair(siIt->second, siIt->first));
		}
		if(mmap.empty())
			continue;
		ofs << it->first;
		for(multimap<int, string>::iterator isIt = mmap.begin(); isIt != mmap.end(); ++isIt)
		{
			ofs << "\t" << isIt->second;
			hmap[it->first].push_back(isIt->second);
		}
		ofs << endl;
	}
	ofs.close();
	//if(!m_word_rword.write(hmap, file))
	//{
	//	cout << "word to rword data write fail!" << endl;
	//	return false;
	//}
	return true;
}
