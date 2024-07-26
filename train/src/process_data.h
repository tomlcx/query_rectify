#ifndef PROCESS_DATA_H
#define PROCESS_DATA_H

#include <set>
#include <map>
#include <fstream>
#include <tr1/unordered_map>
#include "Character.h"
//#include "segmentor.h"
#include "static_hash_vector.h"
#include "encode_process.h"
#include "string_process.h"
#include "pinyin_process.h"
#include "ngram_model.h"
#include "whole_pinyin_create.h"
#include "chword_create.h"
#include "enword_create.h"
using namespace std;

class ProcessData
{
public:
	ProcessData(map<string, string>& config)
	{
		m_in_path = config["in_path"];
		m_out_path = config["out_path"];
		m_ch_thresh = atoi(config["ch_thresh"].c_str());
		m_ch_max_num = atoi(config["ch_max_num"].c_str());
		m_en_thresh = atoi(config["en_thresh"].c_str());
		m_en_max_num = atoi(config["en_max_num"].c_str());
		m_wpy_thresh = atoi(config["wpy_thresh"].c_str());
		m_wpy_max_num = atoi(config["wpy_max_num"].c_str());
		m_word_cooccur_num = atoi(config["word_cooccur_num"].c_str());

		m_nm.init(config);
		m_whole_pinyin_create.init(config);
		m_chword_create.init(config);
		m_enword_create.init(config);

		//m_probSeg.init(m_in_path + "prob/datrie.idx", m_in_path + "prob/gbk_encode.idx",
		//	m_in_path + "prob/ngram_model.idx", m_in_path + "prob/ambi_encode.idx");
	}
	bool processData(string& fname, bool seg = true, string seperator = "\t", int def = 2000);
	bool readPinYin(const string& fname);
	bool readFiltSet(const string& fname);
	void filtCHWord();
	void sortCHENWord();
	bool createIndex();
	bool savePinYin(const string& fname);
	bool savePinYinToWord(const string& fname);
	bool saveWordCooccur(const string& fname);
private:
	vector<string> m_filt;
	vector<int> m_sin_chword;//record single chinese word num
	map<string, set<string> > m_pinyin_chword;
	map<string, set<string> > m_chword_pinyin;
	map<string, int> words2weight;
	tr1::unordered_map<string, int> m_chword, m_enword;
	map<string, int> m_whole_pinyin;
	map<string, map<string, int> > m_word_rword_weight;
	hash_map<string, vector<pair<string, int> > > m_wpy2word, m_fpy2word;
	static_hash_vector<string, vector<pair<string, int> > > m_wpy, m_fpy;
	static_hash_vector<string, vector<string> > m_word_rword;

	map<string, set<string> >::iterator sSIt;
	map<string, string>::iterator ssIt;
	tr1::unordered_map<string, int>::iterator siIt;
	map<string, int>::iterator siIt1;

	class NgramModel m_nm;
	class WholePinYinCreator m_whole_pinyin_create;
	class CHWordCreator m_chword_create;
	class ENWordCreator m_enword_create;

	//segment::B2CProbSegmentor m_probSeg;
	CharacterConverter m_cc;
private:
	string m_in_path;
	string m_out_path;
	int m_ch_thresh;
	int m_ch_max_num;
	int m_en_thresh;
	int m_en_max_num;
	int m_wpy_thresh;
	int m_wpy_max_num;
	int m_word_cooccur_num;
};

#endif
