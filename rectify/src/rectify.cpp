#include "rectify.h"
#include "TimeUtil.h"
#include "session_log_c.h"
#include "code_convert.h"

#define LOG_DIR "log"

Rectify::~Rectify()
{
	if(m_ws != NULL)
	{
		delete m_ws;
		m_ws = NULL;
		cout << "free word segment" << endl;
	}
}

Rectify::Rectify():m_ws(NULL)
{
}

QueryRectify* QueryRectify::create()
{
	QueryRectify* qr = NULL;
	qr = new Rectify();
	return qr;
}

bool Rectify::initSpecial(session_log_c* m_log, FPTR_WRITE_LOG m_logFunc)
{
    m_slog = m_log;
    m_funcWriteLogptr = m_logFunc;
    return true;
}

bool Rectify::init(const string& path)
{
	map<string, string> config;
	read_config(path + "rectify_conf.ini", config);
	config["out_path"] = path + config["out_path"];
	string out_path = config["out_path"];
	string file = out_path + config["out_pinyin"];
	if(!loadPinYin(file))
	{
		cerr << "load pin yin file fail!" << endl;
		return false;
	}
	m_ws = new WordSegmentation(file);
	file = out_path + config["out_word_cooccur"];
	if(!loadWordCooccur(file))
	{
		cerr << "load word cooccur file fail!" << endl;
		return false;
	}
	file = out_path + config["out_near_word"];
	if(!loadNearWord(file))
	{
		cerr << "load near word file fail!" << endl;
		return false;
	}
	file = out_path + "whole_" + config["out_pinyin_word"];
	if(!m_wpy.read(file))
	{
		cerr << "load whole pinyin word fail!" << endl;
		return false;
	}
	file = out_path + "first_" + config["out_pinyin_word"];
	if(!m_fpy.read(file))
	{
		cerr << "load first pinyin word fail!" << endl;
		return false;
	}
	file = out_path + config["out_correct_dict"];
	if(!m_correct_dict.load_serialized_hash_file(file.c_str(), -1))
	{
		cerr << "load correct dict file fail!" << endl;
		return false;
	}
	file = out_path + config["out_wrong2right"];
	if(!m_wrong2right.read(file))
	{
		cerr << "load rectify dict file fail!" << endl;
		return false;
	}

	m_nm.init(config);
	m_chword_create.init(config);
	m_enword_create.init(config);
	m_whole_pinyin_create.init(config);

	if(!m_chword_create.load() || !m_enword_create.load() || !m_whole_pinyin_create.load())
	{
		cerr << "load index fail!" << endl;
		return false;
	}
	if(!m_nm.load())
	{
		cerr << "load ngram model fail!" << endl;
		return false;
	}

	return true;
}

bool Rectify::loadPinYin(const string& filename)
{
	ifstream ifs(filename.c_str());
	if(!ifs)
		return false;
	vector<string> vec;
	string line;
	while(getline(ifs, line))
	{
		trim(line);
		split(line, vec, "\t");
		if(vec.size() < 2 || vec[0] == "")
			continue;
		for(int i = 1; i < vec.size(); ++i)
			m_pinyin_chword[vec[0]].insert(vec[i]);
	}
	ifs.close();
	ifs.clear();

	ifs.open((filename + ".reverse").c_str());
	if(!ifs)
		return false;
	while(getline(ifs, line))
	{
		trim(line);
		split(line, vec, "\t");
		if(vec.size() < 2 || vec[0] == "")
			continue;
		for(int i = 1; i < vec.size(); ++i)
			m_chword_pinyin[vec[0]].insert(vec[i]);
	}
	ifs.close();
	return true;
}

bool Rectify::loadWordCooccur(const string& filename)
{
	ifstream ifs((filename+".txt").c_str());
	if(!ifs)
		return false;
	vector<string> vec;
	string line;
	while(getline(ifs, line))
	{
		trim(line);
		split(line, vec, "\t");
		if(vec.size() < 2)
			continue;
		for(int i = 1; i < vec.size(); ++i)
			m_word_cooccur[vec[0]].insert(vec[i]);
	}
	
	if(!m_word_rword.read(filename))
	{
		cerr << "load word to rword data fail!" << endl;
		return false;
	}
	return true;
}

bool Rectify::loadNearWord(const string& filename)
{
	ifstream ifs(filename.c_str());
	if(!ifs)return false;
	vector<string> vec;
	string line;
	int i, j;
	while(getline(ifs, line))
	{
		trim(line);
		if(line.size() % 2 != 0)continue;
		for(i = 0; i < line.size(); i += 2)
		{
			ushort word = *(ushort*)(line.c_str()+i);
			for(j = 0; j < line.size(); j += 2)
			{
				if(i == j)continue;
				m_word_near[word].push_back(line.substr(j, 2));
			}
		}
	}
	return true;
}

bool Rectify::ifHavSinglePinYin(vector<string>& py)
{
	for(int i = 0; i < py.size(); ++i)
		if(py[i].size() == 1)
			return true;
	return false;
}

bool Rectify::ifHavBlank(string& str, CHARTYPE& ty)
{
	if(ty == ENPUNC && (str[0] == ' ' || str[0] == '?' || str[0] == '.'))
		return true;
	if(ty == CHPUNC && strncmp(str.c_str(), "　", 2) == 0)
		return true;
	return false;
}

bool Rectify::ifRightIndep(string& rword)
{
	if(rword.empty() || isdigit(rword[0]) || isalpha(rword[rword.size()-1]))
		return true;
	return false;
}

void Rectify::delSingleEN(vector<string>& vecWord, vector<CHARTYPE>& vecMark, bool& state)
{
	if(vecMark.size() == 3)
	{
		for(int i = 1; i < vecMark.size() - 1; ++i)
		{
			if(vecMark[i-1] == CHWORD && vecMark[i+1] == CHWORD && vecWord[i-1].size() == 2 &&
			vecWord[i+1].size() == 2 && vecMark[i] == ALPHA && vecWord[i].size() == 1)
			{
				if(!m_chword_create.ifExist(vecWord[i-1] + vecWord[i+1]))return;
				vecWord[i-1] += vecWord[i+1];
				vecWord.erase(vecWord.begin() + i, vecWord.begin() + i + 2);
				vecMark.erase(vecMark.begin() + i, vecMark.begin() + i + 2);
				state = true;
				break;
			}
		}
	}
}

void Rectify::getWordByPinYin(vector<string>& py, vector<vector<ushort> >& chword)
{
	chword.clear();
	chword.resize(py.size());
	for(int i = 0; i < py.size(); ++i)
	{
		if(py[i] == tag[BEG] || py[i] == tag[END] || py[i] == tag[NUM] || py[i] == tag[ALP])
		{
			chword[i].push_back(*(ushort*)&py[i][0]);
			continue;
		}
		set<string>& word = m_pinyin_chword[py[i]];
		for(set<string>::iterator sIt = word.begin(); sIt != word.end(); ++sIt)
			chword[i].push_back(*(ushort*)&(*sIt)[0]);
	}
}

bool Rectify::ifWordCooccur(string& left, string& right)
{
	map<string, set<string> >::iterator it = m_word_cooccur.find(left);
	if(it ==  m_word_cooccur.end() || it->second.find(right) == it->second.end())
		return false;
	return true;
}

bool Rectify::recCHWordByModel(string& ch, string& type, int conf, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	map<string, set<string> >::iterator it = m_word_cooccur.find(type);
	if(it != m_word_cooccur.end())
	{
		if((it->second).find(ch) != (it->second).end())
		{
			rec_word.push_back(make_pair(ch+type, conf));
			return true;
		}
		else if(isalpha(type[0]))
		{
			int diff;
			string str;
			char tmp[30] = {0};
			HASHVECTOR hvec = m_word_rword[type];
			for(int j = 0; j < hvec.count; ++j)
			{
				memcpy(tmp, hvec.data+j*hvec.size, hvec.size);
				str = tmp;
				if(str.size() != ch.size())
					continue;
				diff = edit_distance_ch(str, ch);
				if(diff < 2 && diff < ch.size()/2)
				{
					rec_word.push_back(make_pair(str+type, conf));
					return true;
				}
			}
		}
	}
	return false;
}

bool Rectify::ifSatisForDistance(string& chword, vector<ushort>& code)
{
	if(chword.size()/2 < code.size() || (code.size() == 1 && chword.size()/2 > code.size()))
		return false;
	vector<ushort> v_code;
	getCodeByWord(chword, v_code);
	int ed = edit_distance_sequence(v_code, code);
	int len1 = v_code.size(), len2 = code.size();
	if(len1 == len2)
	{
		if(ed <= 1)
		{
			if(v_code[0] == code[0] && v_code[len1-1] == code[len2-1])
				return true;
		}
		return false;
	}
	else if(len1 == len2 + 1)
	{
		if(ed <= 1)
		{
			int i = 0;
			for(; i < len2; ++i)
			{
				if(v_code[i] != code[i])
					break;
			}
			if(i == len2)
				return true;
		}
		return false;
	}
	if(ed > 1)
		return false;
	return true;
}

bool Rectify::ifAllInForPinYin(string& chword, vector<vector<string> >& py)
{
	if(chword.size()/2 != py[0].size())
		return false;
	string word;
	set<int> set_pos;
	set<string>::iterator sIt;
	bool find;
	for(int i = 0; i < chword.size(); i += 2)
	{
		find = false;
		word = chword.substr(i, 2);
		set<string>& s = m_chword_pinyin[word];
		for(int j = 0; j < py[0].size(); ++j)
		{
			if(s.find(py[0][j]) != s.end())
			{
				find = true;
				set_pos.insert(j);
				break;
			}
		}
		if(!find)
			return false;
	}
	if(set_pos.size() < chword.size()/2)
		return false;
	return find;
}

bool Rectify::recSingleChar(const string& str, vector<CHARTYPE>& vecMark, int cur_pos, string& rec_str)
{
	rec_str = "";
	if(cur_pos == 0 || (!(str.size() == 1 && vecMark[cur_pos] == ALPHA) && 
		!(str.size() == 2 && vecMark[cur_pos] == CHWORD)))
		return false;
	if(vecMark[cur_pos - 1] == NUMBER)
	{
		if(str.size() == 1)
		{
			if(str == "o")
				rec_str = "0";
		}
		else
		{
			if(str == "丅")
				rec_str = "t";
		}
	}
	return !rec_str.empty();
}

void Rectify::getCodeByWord(string& word, vector<ushort>& code)
{
	int len = word.size() / 2;
	code.resize(len);
	memcpy(&code[0], &word[0], sizeof(ushort) * len);
}

int Rectify::getRightAlNum(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, string& rword)
{
	rword = "";
	int ptr = pos;
	for(int i = pos + 1; i < vecWord.size(); ++i)
	{
		if(rword.empty())
		{
			if(ifHavBlank(vecWord[i], vecMark[i]))
				continue;
			else if(vecMark[i] == CHPUNC || vecMark[i] == ENPUNC)
				break;
		}
		if(vecMark[i] == ALPHA)
		{
			ptr = i;
			rword += vecWord[i];
		}
		else if(vecMark[i] == NUMBER)
		{
			ptr = i;
			rword += vecWord[i];
			break;
		}
		else
			break;
	}
	return ptr;
}

bool Rectify::ngramRec(string& rword, vector<vector<string> >& py, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	string res = "";
	int confidence = 0;
	int min_weight = 0xffff;
	vector<vector<ushort> > chword;

	for(int k = 0; k < py.size() && k < 2; ++k)
	{
		getWordByPinYin(py[k], chword);
		m_nm.findBestPath(chword, rec_word);
		if(rec_word.empty())
			continue;
		confidence = (2000+(chword.size()-2) * 35000) / rec_word[0].second;
		if(rec_word[0].first.size() > 2 && !m_chword_create.ifExist(rec_word[0].first))
			continue;
		if(!rec_word.empty() && confidence > 60 && min_weight > rec_word[0].second)
		{
			res = rec_word[0].first;
			min_weight = rec_word[0].second;
		}
	}
	rec_word.clear();
	confidence = (2000+(chword.size()-2) * 35000) / min_weight;
	confidence = confidence > 100 ? 100 : confidence;
	map<string, set<string> >::iterator it;
	if(!res.empty())
	{
		if(ifRightIndep(rword))
		{
			rec_word.push_back(make_pair(res, confidence));
			return true;
		}
		it = m_word_cooccur.find(rword);
		if(it == m_word_cooccur.end() || it->second.find(res) != it->second.end())
		{
			rec_word.push_back(make_pair(res, confidence));
			return true;
		}
	}
	return false;
}

bool Rectify::ngramRec(string& ori_word, string& rword, vector<vector<string> >& py, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	string res = "", tmp = "";
	int confidence = 0;
	int min_weight = 0xffff, tmp_weight = 0xffff;
	int diff = 0;
	vector<vector<ushort> > chword;
	for(int k = 0; k < py.size() && k < 2; ++k)
	{
		getWordByPinYin(py[k], chword);
		m_nm.findBestPath(chword, rec_word);
		for(int pos = 0; pos < rec_word.size(); ++pos)
		{
			confidence = (2000+(chword.size()-2) * 30500) / rec_word[pos].second;
			if(confidence >= 57 && rec_word[pos].first == ori_word)
			{
				rec_word.clear();
				rec_word.push_back(make_pair(ori_word, confidence));
				return true;
			}
			if(pos != 0 && confidence <= 70)
				continue;
			if(confidence <= 60)
				continue;
			if(rec_word[pos].first.size() > 2 && !m_chword_create.ifExist(rec_word[pos].first))
				continue;
			if(min_weight > rec_word[pos].second)
			{
				if(ori_word.size() <= 4)
				{
					diff = edit_distance_fpy(ori_word, rec_word[pos].first);
					if(diff == ori_word.size() / 2)
					{
						if(tmp_weight > rec_word[pos].second)
						{
							tmp = rec_word[pos].first;
							tmp_weight = rec_word[pos].second;
						}
						continue;
					}
				}
				res = rec_word[pos].first;
				min_weight = rec_word[pos].second;
			}
		}
	}
	rec_word.clear();
	if(res == "" && tmp == "")
		return false;
	if(res == "")
	{
		min_weight = tmp_weight;
		res = tmp;
	}
	confidence = (2000+(chword.size()-2) * 30500) / min_weight;
	confidence = confidence > 100 ? 100 : confidence;
	if(ifRightIndep(rword))
	{
		rec_word.push_back(make_pair(res, confidence));
		return true;
	}
	
	map<string, set<string> >::iterator it = m_word_cooccur.find(rword);
	if(it == m_word_cooccur.end() || it->second.find(res) != it->second.end())
	{
		rec_word.push_back(make_pair(res, confidence));
		return true;
	}
	return false;
}

bool Rectify::wholePinYinRec(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, string& rword, string& ori_word, vector<pair<string, int> >& rec_word)
{
	vector<pair<string, int> > result;
	vector<vector<string> > py;
	int weight;
	m_whole_pinyin_create.rectify(ori_word, result);
	for(int i = 0; i < result.size() && i < 5; ++i)
	{
		if(ori_word[0] != result[i].first[0])
			continue;
		m_ws->word_sect_all(result[i].first.c_str(), py);
		weight = result[i].second * 100 / 14000;
		if(weight > 40 && result[i].first == ori_word)
		{
			HASHVECTOR hvec = m_wpy[result[i].first];
			if(getWordFromDict(hvec, rword, rec_word, false))
			{
				if(ifHavAlpha(rec_word[0].first) || ifInCorrectDict(rec_word[0].first) || rec_word[0].first.size() > 10 ||
				 (i + 1 < result.size() && result[i].second * 3 > result[i+1].second))
					return true;
			}
			continue;
		}
		if(i + 1 < result.size() && result[i].second * 3 < result[i+1].second)
			continue;
		if(weight > 60 || ((result.size() == 1 || result[0].second > result[1].second * 2) && i == 0 && ori_word.size() >= 18))
		{
			HASHVECTOR hvec = m_wpy[result[i].first];
			if(getWordFromDict(hvec, rword, rec_word, true))
			{
				int diff = ori_word.size() - result[i].first.size();
				if(diff > 0 && edit_distance_sequence(ori_word, result[i].first) == diff)
				{
					int pos = rec_word[0].first.size() - rword.size();
					if(rword.empty() || isdigit(rword[0]))
						rec_word[0].first.insert(pos, ori_word.substr(result[i].first.size(), diff));
					else
						continue;
				}
				return true;
			}
		}
	}
	return false;
}

bool Rectify::wholePinYinRec(string& rword, vector<string>& pinyin, string& ori_word, string& fpy, vector<pair<string, int> >& result, vector<pair<string, int> >& rec_word)
{
	rec_word.clear();
	vector<string> rec_py;
	vector<vector<string> > rec_wpy;
	int weight, ori_weight = 0;
	if(fpy.size() <= 2 && !result.empty() && result[0].first != ori_word && m_enword_create.ifExist(ori_word))
		return false;
	int cnt = 0;
	for(int i = 0; i < result.size() && i < 5; ++i)
	{
		if(i == 0 && result[i].first == ori_word)
			ori_weight = result[i].second;
		if(ori_word[0] != result[i].first[0])
			continue;
		rec_py.clear();
		if(!m_ws->word_sect_all(result[i].first.c_str(), rec_py) || fpy.size() > rec_py[0].size())
			continue;
		if(fpy.size() < rec_py[0].size() && strncmp(fpy.c_str(), rec_py[0].c_str(), fpy.size()) != 0)
			continue;
		weight = result[i].second;
		//if(i == 0 && ori_weight != 0 && ((fpy.size() <= 4 && weight > 5000) || (fpy.size() > 4 && (result.size() == 1 || weight > 3000))))
		if(i == 0 && ori_weight != 0 && ((fpy.size() <= 4 && weight > 5000 && (result.size() == 1 || weight * 40 > result[i+1].second)) || 
		(fpy.size() > 4 && (result.size() == 1 || (weight * 25 > result[i+1].second && weight > 3000)))))
		{
			HASHVECTOR hvec = m_wpy[result[i].first];
			if(getWordFromDict(hvec, rword, rec_word, false))
			{
				if(ifHavModelInTail(rec_word[0].first))
					return true;
				int len = rec_word[0].first.size() - rword.size();
				string word = rword.empty() ? rec_word[0].first : rec_word[0].first.substr(0, len);
				if(ifInCorrectDict(word) || word.size() > 8 || result.size() == 1 || weight > 80000 ||
				 (i + 1 < result.size() && result[i].second * 4 > result[i+1].second))
					return true;
			}
			continue;
		}
		if(i + 1 < result.size() && result[i].second * 4 < result[i+1].second)
			continue;
		int ed = edit_distance_sequence(fpy, rec_py[0]);
		int idx = 0;
		if(rec_py.size() > 1 &&  edit_distance_sequence(fpy, rec_py[1]) == 0)
		{
			idx = 1;
			ed = 0;
		}
		if(!((ori_weight > 300 && weight > ori_weight * 4 && weight > 30000 && (ed == 0 || ori_weight > 9000)) || 
		(fpy.size() < rec_py[idx].size() && weight > 70000) || 
		(ori_weight == 0 && ed == 0 && ((weight > 20000  && fpy.size() <= 4) || (weight > 600 && fpy.size() > 4))) ||
		(weight > ori_weight * 4 && ed <= 1 && (weight > 500000 || (weight > 3000 && fpy.size() < rec_py[idx].size())) && fpy.size() <= 4)
		))
			continue;
		
		if(fpy.size() == rec_py[idx].size())
		{
			if(ed > 1)
				continue;
			m_ws->word_sect_all(result[i].first.c_str(), rec_wpy);
			bool cont_error = false;
			//理论上两者大小相同，造成两者大小不同是由于中文乱码，具体原因待查明
			if(rec_wpy.size() != rec_py.size())
				return false;
			for(int j = 0; j < pinyin.size() && j < rec_wpy[idx].size(); ++j)
			{
				int k;
				int pos = -2;
				int diff = edit_distance_diff(pinyin[j], rec_wpy[idx][j]);
				if(diff == 0)
					continue;
				int sub = pinyin[j].size() - rec_wpy[idx][j].size();
				if(abs(sub) == 2)
				{
					cont_error = true;
					break;
				}
				int len = sub >= 0 ? rec_wpy[idx][j].size() : pinyin[j].size();
				if(abs(sub) == 1 && diff == 1 && len >= 3)
					continue;
				for(k = 0; k < len; ++k)
				{
					if(pinyin[j][k] != rec_wpy[idx][j][k])
					{
						if(k != pos + 1)
							pos = k;
						else
						{
							cont_error = true;
							break;
						}
					}
				}
				if(abs(sub) == 1 && pos == len - 1 && diff > 1)
					cont_error = true;
				if(cont_error)
					break;
			}
			if(cont_error)
				continue;
		}

		HASHVECTOR hvec = m_wpy[result[i].first];
		if(getWordFromDict(hvec, rword, rec_word, true))
		{
			if(!((rword.empty() || isdigit(rword[0])) && ifHavAlpha(rec_word[0].first) && result[i].first.size() > ori_word.size()+1))
			{
				if(ed == 1 && fpy.size() <= 4)rec_word[0].second = 61;
				return true;
			}
		}
	}
	return false;
}

bool Rectify::firstPinYinRec(string& py, string& rword, vector<pair<string, int> >& rec_word)
{
	if(py.size() <= 2 && (rword.empty() || isdigit(rword[0])))
		return false;
	HASHVECTOR hvec = m_fpy[py];
	if(getWordFromDict(hvec, rword, rec_word, true) && rec_word[0].second > 61)
		return true;
	return false;
}

bool Rectify::getWordFromDict(HASHVECTOR& hvec, string& rword, vector<pair<string, int> >& rec_word, bool ifchange)
{
	rec_word.clear();
	if(hvec.count <= 0)
		return false;
	char word[30] = {0}, *p;
	int weight = 0;
	unsigned long long conf = 0;
	bool rword_exist = rword.empty() ? false : true;
	int str_len = hvec.size - sizeof(int);
	int num_pos = 0, cur_pos = 0, numb_len = 0;
	map<string, set<string> >::iterator it;
	for(int i = 0; i < hvec.count && i < 2; ++i)
	{
		memcpy(&word, hvec.data+i*hvec.size, str_len);
		memcpy(&weight, hvec.data+i*hvec.size+str_len, sizeof(int));
		p = word;
		while(*p != '\0')
		{
			if(*p > 0)
				break;
			p += 2;
		}
		conf = weight * 100 / 120000;
		conf = conf <= 60 ? 61 : conf;
		conf = conf > 100 ? 100 : conf;
		if(!rword_exist)
		{
			rec_word.push_back(make_pair(word, conf));
			return true;
		}
		string ch = string(word, p);
		string en = string(p, word+strlen(word));
		if(!en.empty() && isalpha(rword[0]))
			continue;
		string type = en + rword;
		num_pos = en.size();
		for(int j = 0; j < rword.size(); ++j)
		{
			if(!isdigit(rword[j]))
				continue;
			num_pos += j;
			break;
		}
		numb_len = type.size() - num_pos;
		if((i == 0 && (ch.size() > 10 || weight <= 15000)) || isdigit(type[0]) || isalpha(type[type.size()-1]))
		{
			rec_word.push_back(make_pair(ch+type, conf));
			return true;
		}
		it = m_word_cooccur.find(ch);
		if(!ifchange && it != m_word_cooccur.end())
		{
			if(it->second.find(type) != it->second.end())
			{
				rec_word.push_back(make_pair(ch+type, 100));
				return true;
			}
			int diff;
			string str;
			char tmp[30] = {0};
			HASHVECTOR hvec = m_word_rword[ch];
			for(int j = 0; j < hvec.count; ++j)
			{
				memcpy(tmp, hvec.data+j*hvec.size, hvec.size);
				str = tmp;
				if(abs((int)str.size() - (int)type.size()) > 1 || isdigit(str[0]) || isalpha(str[str.size()-1]))
					continue;
				diff = edit_distance(str, type);
				if(diff >= 2)
					continue;
				cur_pos = str.size() - numb_len;
				if(cur_pos > 0 && !isdigit(str[cur_pos-1]) && edit_distance(str.substr(cur_pos), type.substr(num_pos)) == 0)
				{
					rec_word.push_back(make_pair(ch+str, conf));
					return true;
				}
			}
		}
		if(recCHWordByModel(ch, type, conf, rec_word))
			return true;
	}
	return false;
}

bool Rectify::chwordRec(vector<vector<string> >& all_py, vector<ushort>& v_code,
			 const string& ori_word, string& rword, vector<pair<string, int> >& rec_word)
{
	int weight, ori_weight = 0, pos = 0;
	string str;
	m_chword_create.rectify(ori_word, rec_word);
	if(rec_word.empty())
		return false;
	weight = rec_word[pos].second * 100 / 130000;
	if(rec_word[pos].first == ori_word)
	{
		if(weight > 15)
		{
			rec_word.clear();
			rec_word.push_back(make_pair(ori_word, weight));
			return true;
		}
		ori_weight = rec_word[pos].second;
		++pos;
	}
	for(int i = pos; i < rec_word.size(); ++i)
	{
		if(ori_word.size() != rec_word[i].first.size())
			continue;
		weight = rec_word[i].second * 100 / 90000;
		if(weight > 35 && ifAllInForPinYin(rec_word[i].first, all_py))
		{
			weight = weight <= 60 ? 61 : weight;
			weight = weight > 100 ? 100 : weight;
			str = rec_word[i].first;
			rec_word.clear();
			rec_word.push_back(make_pair(str, weight));
			return true;
		}
	}
	map<string, set<string> >::iterator it;
	bool hav = (!rword.empty() && isalpha(rword[0])) ? true : false;
	for(int i = pos; i < rec_word.size(); ++i)
	{
		weight = rec_word[i].second * 100 / 120000;
		if((rec_word[i].first.size() <= 8 && weight <= 70) || (rec_word[i].first.size() > 8 && weight <= 60))
			continue;
		if(hav)
		{
			it = m_word_cooccur.find(rword);
			if(it == m_word_cooccur.end() || it->second.find(rec_word[i].first) == it->second.end())
				continue;
		}
		if(!ifSatisForDistance(rec_word[i].first, v_code))
			continue;
		str = rec_word[i].first;
		rec_word.clear();
		weight = weight > 100 ? 100 : weight;
		rec_word.push_back(make_pair(str, weight));
		return true;
	}
	return false;
}

bool Rectify::enwordRec(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, vector<pair<string, int> >& rec_word)
{
	int weight = 0, ori_weight = 0, confidence;
	vector<vector<string> > py;
	m_enword_create.rectify(vecWord[pos], rec_word);
	map<string, set<string> >::iterator it;
	for(int i = 0; i < rec_word.size(); ++i)
	{
		confidence = rec_word[i].second * 100 / 10000;
		if(i == 0 && confidence > 40 && rec_word[i].first == vecWord[pos])
		{
			int thresh = rec_word[i].second * 4;
			if((rec_word.size() > i + 2 && (rec_word[i].second < 20000 || rec_word[i].first.size() < rec_word[i+2].first.size()) && thresh < rec_word[i+2].second) || 
			(rec_word.size() > i + 1 && thresh < rec_word[i+1].second))
				continue;
			rec_word.clear();
			rec_word.push_back(make_pair(vecWord[pos], confidence));
			return true;
		}
		if((i != 1 || rec_word[i].second < 12000) && (i + 1 < rec_word.size() && rec_word[i].second * 2 < rec_word[i+1].second))
			continue;
		if(confidence > 60)
		{
			string word = rec_word[i].first;
			bool havDep = true;
			if(pos + 1 < vecMark.size() && vecMark[pos+1] == NUMBER)
			{
				int diff = vecWord[pos].size() - word.size();
				string en;
				if(diff > 0 && edit_distance_sequence(vecWord[pos], word) == diff)
					en = vecWord[pos].substr(word.size(), diff);
				it = m_word_cooccur.find(word);
				if(!en.empty())
				{
					vector<string> fpy;
					if(m_ws->word_sect_all(word.c_str(), fpy))
					{
						HASHVECTOR hvec = m_wpy[word];
						if(getWordFromDict(hvec, en, rec_word, true))
							return true;
						return false;
					}
					else
						word += ' ' + en;
				}
				else if(it == m_word_cooccur.end() || (it->second).find(vecWord[pos+1]) == (it->second).end())
					continue;
			}
			else if(pos + 1 < vecMark.size() && (vecMark[pos+1] == CHWORD && vecWord[pos+1].size() <= 8))
			{
				it = m_word_cooccur.find(word);
				if(it == m_word_cooccur.end() || (it->second).find(vecWord[pos+1]) == (it->second).end())
					continue;
			}
			else if((pos == 1 || (pos > 1 && vecMark[pos-2] != NUMBER)) && vecMark[pos-1] == CHWORD)
			{
				it = m_word_cooccur.find(word);
				if(it == m_word_cooccur.end() || (it->second).find(vecWord[pos-1]) == (it->second).end())
					continue;
			}
			else if(pos > 0 && vecMark[pos-1] == NUMBER)
			{
				it = m_word_cooccur.find(vecWord[pos-1]);
				if(it == m_word_cooccur.end() || (it->second).find(vecWord[pos]) == (it->second).end())
					continue;
			}
			else
				havDep = false;
			if(m_ws->word_sect_all(word.c_str(), py) && (py[0].size() > 2 || !havDep) && !ifHavSinglePinYin(py[0]))
				continue;
			int diff = edit_distance_diff(word, vecWord[pos]);
			if(diff > 0 && vecWord[pos].size() <= 4 && !havDep && 
			((pos == 0 && m_fpy[vecWord[pos]].count > 0) || edit_distance(vecWord[pos], word) == 2))
				return false;
			
			confidence = confidence > 100 ? 100 : confidence;
			rec_word.clear();
			rec_word.push_back(make_pair(word, confidence));
			return true;
		}
	}
	return false;
}

bool Rectify::nearwordRec(const string& word, vector<vector<string> >& whole_pinyin, vector<pair<string, int> >& rec_word)
{
	//word为纯中文,不包含英文字母
	rec_word.clear();
	if(word.size() % 2 != 0)return false;
	if(whole_pinyin.empty())
		getWholePinYin(word, m_chword_pinyin, whole_pinyin);
	//if(whole_pinyin.empty() || word.size() / 2 != whole_pinyin[0].size())return false;
	int i, j, k, pos, ori_weight = 0, weight = 0;
	ushort code;
	string pinyin;
	char wd[30] = {0};
	if(!whole_pinyin.empty())
	{
		if(word.size() / 2 != whole_pinyin[0].size())return false;
		for(i = 0; i < whole_pinyin[0].size(); ++i)
			pinyin += whole_pinyin[0][i];
		HASHVECTOR hvec = m_wpy[pinyin];
		int str_len = hvec.size - sizeof(int);
		for(pos = 0; pos < hvec.count && pos < 3; ++pos)
		{
			memcpy(&wd, hvec.data + pos * hvec.size, str_len);
			memcpy(&weight, hvec.data + pos * hvec.size + str_len, sizeof(int));
			if(strcmp(wd, word.c_str()) == 0)
			{
				ori_weight = weight;
				break;
			}
		}
	}
	vector<string> py;
	map<ushort, vector<string> >::iterator it;
	for(i = 0; i < word.size(); i += 2)
	{
		code = *(ushort*)(word.c_str()+i);
		it = m_word_near.find(code);
		if(it == m_word_near.end())continue;
		vector<string>& near_word = it->second;
		string tmp_word1 = word.substr(0, i);
		string tmp_word2 = word.substr(i + 2);
		vector<string> pinyin1, pinyin2;
		if(!getWholePinYin(tmp_word1, m_chword_pinyin, pinyin1) || pinyin1.empty())
			continue;
		if(!getWholePinYin(tmp_word2, m_chword_pinyin, pinyin2) || pinyin2.empty())
			continue;
		for(j = 0; j < near_word.size(); ++j)
		{
			pinyin = "";
			string tmp_word = tmp_word1 + near_word[j] + tmp_word2;
			for(k = 0; k < word.size(); k += 2)
			{
				//if(i != k)
				//{
				//	pinyin += whole_pinyin[0][k/2];
				//}
				//else
				//{
				//	if(!getWholePinYin(near_word[j], m_chword_pinyin, py) || py.empty())
				//		break;
				//	pinyin += py[0];
				//}
				if(i == k)
				{
					if(!getWholePinYin(near_word[j], m_chword_pinyin, py) || py.empty())
						break;
					pinyin = pinyin1[0] + py[0] + pinyin2[0];
					break;
				}
			}
			if(pinyin == "")continue;
			HASHVECTOR hvec = m_wpy[pinyin];
			int str_len = hvec.size - sizeof(int);
			for(pos = 0; pos < hvec.count && pos < 3; ++pos)
			{
				memcpy(&wd, hvec.data + pos * hvec.size, str_len);
				memcpy(&weight, hvec.data + pos * hvec.size + str_len, sizeof(int));
				if(strcmp(wd, tmp_word.c_str()) == 0)
				{
					if(rec_word.empty())
					{
						if(weight > ori_weight)
							rec_word.push_back(make_pair(tmp_word, weight));
					}
					else if(weight > rec_word[0].second)
					{
						rec_word[0].first = tmp_word;
						rec_word[0].second = weight;
					}
					break;
				}
			}
		}
	}
	if(!rec_word.empty())
	{
		rec_word[0].second = 60 + rec_word[0].second / (1 + ori_weight);
		return true;
	}
	return false;
}

void Rectify::processRec(vector<pair<string, int> >& rec_word, string& ori_word, int& confidence, string& res, bool& state)
{
	if(rec_word[0].first != ori_word)
	{
		state = true;
		int weight = rec_word[0].second;
		confidence = confidence > weight ? weight : confidence;
	}
	res += rec_word[0].first;
}

bool Rectify::ifInCorrectDict(const string& ori_str)
{
	return m_correct_dict[ori_str] != -1;
}

bool Rectify::ifInRectifyDict(const string& ori_str, vector<pair<string, int> >& rec)
{
	rec.clear();
	HASHVECTOR hvec = m_wrong2right[ori_str];
	char tmp[30] = {0};
	int str_len;
	int weight;
	if(hvec.count > 0)
	{
		str_len = hvec.size - sizeof(int);
		memcpy(tmp, hvec.data, str_len);
		memcpy(&weight, hvec.data + str_len, sizeof(int));
		rec.push_back(make_pair(tmp, weight));
		return true;
	}
	return false;
}

bool Rectify::rectify(const string& query, vector<pair<string, int> >& rec)
{
    string utf8_query;
    code_convert("gbk", "utf8", query, utf8_query);

	rec.clear();
	if(query.size() > 30)
	{
	    COMMON_LOG(L_NOTICE, "[analysis] q=%s|query size too long", utf8_query.c_str());
		return false;
	}

	string ori_word = query;
	trim(ori_word);
	to_lower(ori_word);
	m_cc.Convert_t2s(const_cast<char*>(ori_word.c_str()));

	if(ifInCorrectDict(ori_word))
	{
	    COMMON_LOG(L_NOTICE, "[analysis] q=%s|whitelist", utf8_query.c_str());
		return true;
	}
	//默认纠错的词和原词是不同的，这里就不检查了
	if(ifInRectifyDict(ori_word, rec))
	{
	    COMMON_LOG(L_NOTICE, "[analysis] q=%s|wrong2right", utf8_query.c_str());
		return true;
	}

	string str, wordtmp = ori_word;
	vector<string> vecWord;
	vector<CHARTYPE> vecMark;
	vector<pair<string, int> > rec_word;
	delquestionmark(wordtmp);
	bool ischange = false;
	if(wordtmp != ori_word)
	{
		splitCHAlNum(wordtmp, vecWord, vecMark);
		ischange = true;
	}
	else
	{
		splitCHAlNum(ori_word, vecWord, vecMark);
	}

	std::stringstream info;
	if(vecWord.size() > 8)
	{
	    info << "|split word size too lang";
	    COMMON_LOG(L_NOTICE, "[analysis] q=%s|%s", utf8_query.c_str(), info.str().c_str());
		return false;
	}
	int confidence = 100, start = -1;
	vector<vector<string> > whole_pinyin;

	delSingleEN(vecWord, vecMark, ischange);

	vector<string> vecWordUtf8;
	for (int i=0; i< vecWord.size(); i++)
	{
	    string ss;
	    code_convert("gbk", "utf8", vecWord[i], ss);
	    vecWordUtf8.push_back(ss);
	    info << ss;
	    if (i != vecWord.size()-1)
	    {
	        info << ",";
	    }
	}

	for(int i = 0; i < vecMark.size(); ++i)
	{
	    if(whole_pinyin.empty() && (vecMark[i] == ALPHA || vecMark[i] == CHWORD))
	    {
	        // test
	        if (ifInCorrectDict(vecWord[i])) {
	            info << "|CHECK " << vecWordUtf8[i] << " whitelist";
	            str += vecWord[i];
	            continue;
	        }
	        if(ifInRectifyDict(vecWord[i], rec_word))
	        {
	            processRec(rec_word, vecWord[i], confidence, str, ischange);
	            info << "|CHECK " << vecWordUtf8[i] << " wrong2right [" << confidence << "]";
	            continue;
	        }
	    }

	    vector<vector<string> > py;
	    if(vecMark[i] == ALPHA)
	    {
	        info << "|ALPHA " << vecWordUtf8[i];
	        if(vecWord[i].size() <= 1)
	        {
	            string tmp;
	            if(recSingleChar(vecWord[i], vecMark, i, tmp))
	            {
	                ischange = true;
	                str += tmp;
	                confidence = confidence > 90 ? 90 : confidence;
	            }
	            else
	                str += vecWord[i];

	            info << "|size<=1 [" << confidence << "]";
	            continue;
	        }
	        if(i > 0 && vecMark[i-1] == NUMBER)
	        {
	            if(ifWordCooccur(vecWord[i-1], vecWord[i]))
	            {
	                info << "|word cooccur";
	                str += vecWord[i];
	                start = -1;
	                whole_pinyin.clear();
	                continue;
	            }
	        }
	        if(whole_pinyin.empty())
	        {
	            info << "|whole_pinyin empty";
	            if(ifInCorrectDict(vecWord[i]))
	            {
	                info << "whitelist";
	                str += vecWord[i];
	                continue;
	            }
	            m_ws->word_sect_all(vecWord[i].c_str(), whole_pinyin);
	        }
	        if(!whole_pinyin.empty())
	        {
	            info << "|whole_pinyin not empty";
	            if(start == -1)
	            {
	                start = i;
	                int times = whole_pinyin.size() > 2 ? 2 : whole_pinyin.size();
	                whole_pinyin.erase(whole_pinyin.begin()+times, whole_pinyin.end());
	                info << ",start";
	            }
	            if(i + 1 < vecMark.size() && vecMark[i+1] == CHWORD)
	            {
	                if(vecWord[i+1].size() < 16 && vecWord[i+1] != "丅")
	                {
	                    info << ",next not 丅";
	                    continue;
	                }
	            }
	            else if(i + 2 < vecMark.size() && ifHavBlank(vecWord[i+1], vecMark[i+1]) && vecMark[i+2] == ALPHA &&
	            vecWord[i+2].size() > 1 && m_ws->word_sect_all(vecWord[i+2].c_str(), py))
	            {
	                info << ",ifHavBlank";
	                //暂且只用第一种拼音切分
	                for(int j = 0; j < whole_pinyin.size(); ++j)
	                    whole_pinyin[j].insert(whole_pinyin[j].end(), py[0].begin(), py[0].end());
	                ++i;
	                continue;
	            }
	            //全拼
	            int j;
	            string rword;
	            int end = getRightAlNum(vecWord, vecMark, i, rword);
	            if(start == 0 || (start > 0 && (vecMark[start-1] == CHPUNC || vecMark[start-1] == ENPUNC)))
	            {
	                info << ",wholePinYinRec";
	                string tmp_py;
	                vector<pair<string, int> > result;
	                for(j = 0; j < whole_pinyin.size(); ++j)
	                {
	                    string ori_py, fpy;
	                    for(int k = 0; k < whole_pinyin[j].size(); ++k)
	                    {
	                        ori_py += whole_pinyin[j][k];
	                        fpy += whole_pinyin[j][k][0];
	                    }
	                    if(tmp_py != ori_py)
	                        m_whole_pinyin_create.rectify(ori_py, result);
	                    if(!wholePinYinRec(rword, whole_pinyin[j], ori_py, fpy, result, rec_word))
	                    {
	                        tmp_py = ori_py;
	                        continue;
	                    }
	                    string ori_word;
	                    for(int k = start; k <= end; ++k)
	                        if(vecMark[k] != CHPUNC && vecMark[k] != ENPUNC)
	                            ori_word += vecWord[k];
	                        processRec(rec_word, ori_word, confidence, str, ischange);
	                        info << " [" << confidence << "]";
	                        i = end;
	                        break;
	                }
	                if(j < whole_pinyin.size())
	                {
	                    info << ",end";
	                    start = -1;
	                    whole_pinyin.clear();
	                    continue;
	                }
	            }
	            //N元模型
	            for(j = 0; j < whole_pinyin.size(); ++j)
	            {
	                whole_pinyin[j].insert(whole_pinyin[j].begin(), tag[BEG]);
	                if(start > 0)
	                {
	                    if(vecMark[start-1] == NUMBER)
	                        whole_pinyin[j].insert(whole_pinyin[j].begin()+1, tag[NUM]);
	                    else if(vecMark[start-1] == ALPHA)
	                        whole_pinyin[j].insert(whole_pinyin[j].begin()+1, tag[ALP]);
	                }
	                whole_pinyin[j].push_back(tag[END]);
	            }
	            if(ngramRec(rword, whole_pinyin, rec_word))
	            {
	                processRec(rec_word, vecWord[i], confidence, str, ischange);
	                info << ",ngramRec [" << confidence << "]";
	            }
	            else if(start == i && enwordRec(vecWord, vecMark, i, rec_word))	//英文
                {
	                processRec(rec_word, vecWord[i], confidence, str, ischange);
	                info << ",enwordRec [" << confidence << "]";
                }
	            else
	            {
	                info << ",none";
	                for(int j = start; j <= i; ++j)
	                    str += vecWord[j];
	            }
	            start = -1;
	            whole_pinyin.clear();
	            continue;
	        }
	        //英文
	        if(enwordRec(vecWord, vecMark, i, rec_word))
	        {
	            processRec(rec_word, vecWord[i], confidence, str, ischange);
	            info << "|enwordRec [" << confidence << "]";
	            continue;
	        }
	        //全拼
	        string rword;
	        int end = getRightAlNum(vecWord, vecMark, i, rword);
	        if(wholePinYinRec(vecWord, vecMark, i, rword, vecWord[i], rec_word))
	        {
	            processRec(rec_word, vecWord[i], confidence, str, ischange);
	            info << "|wholePinYinRec [" << confidence << "]";
	            i = end;
	            continue;
	        }
	        //首拼
	        if(i == 0 && firstPinYinRec(vecWord[i], rword, rec_word))
	        {
	            processRec(rec_word, vecWord[i], confidence, str, ischange);
	            info << "|firstPinYinRec [" << confidence << "]";
	            i = end;
	            continue;
	        }
	        str += vecWord[i];
	    }
	    else if(vecMark[i] == CHWORD)
	    {
	        info << "|CHWORD " << vecWordUtf8[i];
	        if(vecWord[i].size() >= 16 || (vecWord[i].size() >= 12 && i > 0 && vecMark[i-1] == NUMBER))
	        {
	            info << "|size>=16";
	            str += vecWord[i];
	            continue;
	        }
	        else if(vecWord[i].size() <= 2)
	        {
	            string tmp;
	            //以下添加满足whole_pinyin肯定为空
	            if(recSingleChar(vecWord[i], vecMark, i, tmp))
	            {
	                ischange = true;
	                str += tmp;
	                confidence = confidence > 90 ? 90 : confidence;
	                info << "|recSingleChar [" << confidence << "]";
	                continue;
	            }
	        }
	        if(whole_pinyin.empty())
	        {
	            info << "|whole_pinyin empty";
	            if(getWholePinYin(vecWord[i], m_chword_pinyin, whole_pinyin))
	            {
	                info << ",getWholePinYin true";
	                start = i;
	                int times = whole_pinyin.size() > 2 ? 2 : whole_pinyin.size();
	                whole_pinyin.erase(whole_pinyin.begin()+times, whole_pinyin.end());
	            }
	            else
	            {
	                info << ",getWholePinYin false";
	                str += vecWord[i];
	                continue;
	            }
	        }
	        else
	        {
	            info << "|whole_pinyin not empty";
	            //暂且只用第一种拼音组合
	            if(getWholePinYin(vecWord[i], m_chword_pinyin, py))
	            {
	                info << ",getWholePinYin true";
	                for(int j = 0; j < whole_pinyin.size(); ++j)
	                    whole_pinyin[j].insert(whole_pinyin[j].end(), py[0].begin(), py[0].end());
	            }
	            else
	            {
	                info << ",getWholePinYin false";
	                for(int j = start; j <= i; ++j)
	                    str += vecWord[j];
	                continue;
	            }
	        }
	        if(i + 1 < vecMark.size() && vecMark[i+1] == ALPHA && !ifInCorrectDict(vecWord[i+1]) &&
	        vecWord[i+1].size() > 1 && m_ws->word_sect_all(vecWord[i+1].c_str(), py))
	        {
	            info << "|word_sect_all_1";
	            //暂且只用第一种拼音切分
	            for(int j = 0; j < whole_pinyin.size(); ++j)
	                whole_pinyin[j].insert(whole_pinyin[j].end(), py[0].begin(), py[0].end());
	            continue;
	        }
	        if(i + 2 < vecMark.size() && ifHavBlank(vecWord[i+1], vecMark[i+1]) && vecMark[i+2] == ALPHA &&
	        !ifInCorrectDict(vecWord[i+2]) && vecWord[i+2].size() > 1 && m_ws->word_sect_all(vecWord[i+2].c_str(), py))
	        {
	            info << "|word_sect_all_2";
	            //暂且只用第一种拼音切分
	            for(int j = 0; j < whole_pinyin.size(); ++j)
	                whole_pinyin[j].insert(whole_pinyin[j].end(), py[0].begin(), py[0].end());
	            continue;
	        }

	        string rword;
	        int end = getRightAlNum(vecWord, vecMark, i, rword);
	        if(start == i && (vecWord[i].size() == 2 || (ifRightIndep(rword) && ifInCorrectDict(vecWord[i]))))
	        {
	            info << "|ifRightIndep";
	            str += vecWord[i];
	            whole_pinyin.clear();
	            start = -1;
	            continue;
	        }

	        //全拼
	        int j;
	        if(start == 0 || (start > 0 && (vecMark[start-1] == CHPUNC || vecMark[start-1] == ENPUNC)))
	        {
	            info << "|wholePinYinRec";
	            string tmp_py;
	            vector<pair<string, int> > result;
	            for(j = 0; j < whole_pinyin.size(); ++j)
	            {
	                string ori_py, fpy;
	                for(int k = 0; k < whole_pinyin[j].size(); ++k)
	                {
	                    ori_py += whole_pinyin[j][k];
	                    fpy += whole_pinyin[j][k][0];
	                }
	                if(tmp_py != ori_py)
	                    m_whole_pinyin_create.rectify(ori_py, result);
	                if(!wholePinYinRec(rword, whole_pinyin[j], ori_py, fpy, result, rec_word))
	                {
	                    if(start == i && !rword.empty() && recCHWordByModel(vecWord[i], rword, 61, rec_word))
	                    {
	                        string ori;
	                        for(int k = i; k <= end; ++k)
	                            if(vecMark[k] != CHPUNC && vecMark[k] != ENPUNC)
	                                ori += vecWord[k];
	                            processRec(rec_word, ori, confidence, str, ischange);
	                            info << ",recCHWordByModel [" << confidence << "]";
	                            i = end;
	                            break;
	                    }
	                    tmp_py = ori_py;
	                    continue;
	                }
	                if(start == i && vecWord[i].size() <= 4)
	                {
	                    string tmp;
	                    for(int k = 0; k < rec_word[0].first.size(); ++k)
	                    {
	                        if(rec_word[0].first[k] < 0)
	                            tmp += rec_word[0].first[k++];
	                        else
	                            break;
	                        tmp += rec_word[0].first[k];
	                    }
	                    int diff = edit_distance_ch(tmp, vecWord[i]);
	                    vector<string> tmp_wpy;
	                    if(diff > 0 && getWholePinYin(tmp, m_chword_pinyin, tmp_wpy))
	                    {
	                        if(tmp_wpy[0] == ori_py)
	                        {
	                            bool find = false;
	                            if(ifInCorrectDict(vecWord[i]))
	                                find = true;
	                            if(!find)
	                            {
	                                HASHVECTOR hvec = m_wpy[ori_py];
	                                int str_len = hvec.size - sizeof(int), weight, max_weight = 0;
	                                char word[30] = {0};
	                                for(int pos = 0; pos < hvec.count && pos < 3; ++pos)
	                                {
	                                    memcpy(&word, hvec.data + pos * hvec.size, str_len);
	                                    memcpy(&weight, hvec.data + pos * hvec.size + str_len, sizeof(int));
	                                    if(strcmp(word, vecWord[i].c_str()) == 0 && weight * 2 > max_weight)
	                                    {
	                                        find = true;
	                                        break;
	                                    }
	                                    if(pos == 0)
	                                        max_weight = weight;
	                                }
	                            }
	                            if(find)
	                            {
	                                if(rword.empty())
	                                    rec_word[0].first = vecWord[i];
	                                else
	                                {
	                                    map<string, set<string> >::iterator it = m_word_cooccur.find(vecWord[i]);
	                                    if(it != m_word_cooccur.end() && it->second.find(rword) != it->second.end())
	                                        rec_word[0].first = vecWord[i] + rword;
	                                }
	                            }
	                        }
	                        else if(diff >= 2 && ifRightIndep(rword))
	                            continue;
	                    }
	                }
	                string ori;
	                for(int k = start; k <= end; ++k)
	                    if(vecMark[k] != CHPUNC && vecMark[k] != ENPUNC)
	                        ori += vecWord[k];
	                    processRec(rec_word, ori, confidence, str, ischange);
	                    info << " [" << confidence << "]";
	                    i = end;
	                    break;
	            }
	            if(j < whole_pinyin.size())
	            {
	                info << ",end";
	                whole_pinyin.clear();
	                start = -1;
	                continue;
	            }
	        }
	        //N元模型
	        if(i == vecMark.size()-1 || (vecMark[i+1] != NUMBER))
	        {
	            info << "|ngram";
	            for(j = 0; j < whole_pinyin.size(); ++j)
	            {
	                whole_pinyin[j].insert(whole_pinyin[j].begin(), tag[BEG]);
	                if(start > 0)
	                {
	                    if(vecMark[start-1] == NUMBER)
	                        whole_pinyin[j].insert(whole_pinyin[j].begin()+1, tag[NUM]);
	                    else if(vecMark[start-1] == ALPHA)
	                        whole_pinyin[j].insert(whole_pinyin[j].begin()+1, tag[ALP]);
	                }
	                whole_pinyin[j].push_back(tag[END]);
	            }
	            if((start == i && ngramRec(vecWord[i], rword, whole_pinyin, rec_word)) ||
	            (start != i && ngramRec(rword, whole_pinyin, rec_word)))
	            {
	                processRec(rec_word, vecWord[i], confidence, str, ischange);
	                info << ",ngramRec [" << confidence << "]";
	                start = -1;
	                whole_pinyin.clear();
	                continue;
	            }

	            for(j = 0; j < whole_pinyin.size(); ++j)
	            {
	                whole_pinyin[j].erase(whole_pinyin[j].begin()+whole_pinyin[j].size()-1);
	                if(start > 0)
	                {
	                    if(vecMark[start-1] == NUMBER || vecMark[start-1] == ALPHA)
	                        whole_pinyin[j].erase(whole_pinyin[j].begin()+1);
	                }
	                whole_pinyin[j].erase(whole_pinyin[j].begin());
	            }
	        }
	        for(j = start; j < i; ++j)
	            str += vecWord[j];

	        //中文纠错
	        if(vecWord.size() == 1)
	        {
	            vector<ushort> v_code;
	            getCodeByWord(vecWord[i], v_code);
	            if(chwordRec(whole_pinyin, v_code, vecWord[i], rword, rec_word))
	            {
	                start = -1;
	                whole_pinyin.clear();
	                processRec(rec_word, vecWord[i], confidence, str, ischange);
	                info << "|chwordRec [" << confidence << "]";
	                continue;
	            }
	        }
	        start = -1;
	        whole_pinyin.clear();
	        str += vecWord[i];
	    }
	    else
	    {
	        info << "|OTHER " << vecWordUtf8[i];
            str += vecWord[i];
        }
	}
	if(ifInRectifyDict(str, rec))
	{
	    info << "|end whitelist";
	    COMMON_LOG(L_NOTICE, "[analysis] q=%s|%s", utf8_query.c_str(), info.str().c_str());
		if(ori_word == rec[0].first)rec.clear();
		return true;
	}
	if(vecWord.size() == 1 && vecMark[0] == CHWORD)
	{
		if(nearwordRec(str, whole_pinyin, rec_word))
		{
			str = "";
			processRec(rec_word, vecWord[0], confidence, str, ischange);
			info << "|end nearwordRec [" << confidence << "]";
		}
	}

	COMMON_LOG(L_NOTICE, "[analysis] q=%s|%s", utf8_query.c_str(), info.str().c_str());
	if(ischange)
		rec.push_back(make_pair(str, confidence));
	return true;
}
