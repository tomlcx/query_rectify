#ifndef RECTIFY_H
#define RECTIFY_H
//#define DEBUG

#include "query_rectify.h"

#include "Character.h"
#include "parse_config.h"
#include "encode_process.h"
#include "string_process.h"
#include "pinyin_process.h"
#include "word_segment.h"
#include "ngram_model.h"
#include "chword_create.h"
#include "enword_create.h"
#include "whole_pinyin_create.h"
#include "static_hash_vector.h"
#include <sstream>

#define COMMON_LOG(level, fmt, ...)  (m_funcWriteLogptr(m_slog, true, level, __FILE__, __LINE__,  fmt, ##__VA_ARGS__));
#define SESSION_LOG(level, fmt, ...)  (m_funcWriteLogptr(m_slog, false, level, __FILE__, __LINE__,  fmt, ##__VA_ARGS__));

class Rectify:public QueryRectify
{
public:
	bool init(const string& path);
	bool initSpecial(session_log_c* m_log, FPTR_WRITE_LOG m_logFunc);
	bool rectify(const string& query, vector<pair<string, int> >& rec);
	bool loadPinYin(const string& filename);
	bool loadWordCooccur(const string& filename);
	bool loadNearWord(const string& filename);

	void getWordByPinYin(vector<string>& py, vector<vector<ushort> >& chword);
	bool recCHWordByModel(string& ch, string& type, int conf, vector<pair<string, int> >& rec_word);
	int getRightAlNum(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, string& rword);
	bool ifWordCooccur(string& left, string& right);
	bool ifHavSinglePinYin(vector<string>& py);
	bool ifHavBlank(string& str, CHARTYPE& ty);
	bool ifRightIndep(string& rword);
	bool ifAllInForPinYin(string& chword, vector<vector<string> >& py);
	bool ifSatisForDistance(string& chword, vector<ushort>& code);
	bool ifSatisForPinYin(string& chword, vector<string>& pinyin);
	void getCodeByWord(string& word, vector<ushort>& code);
	bool ifInCorrectDict(const string& ori_str);
	bool ifInRectifyDict(const string& ori_str, vector<pair<string, int> >& rec);
	void delSingleEN(vector<string>& vecWord, vector<CHARTYPE>& vecMark, bool& state);
	bool getWordFromDict(HASHVECTOR& hvec, string& rword, vector<pair<string, int> >& rec_word, bool ifchange = false);

	bool ngramRec(string& rword, vector<vector<string> >& py, vector<pair<string, int> >& rec_word);
	bool ngramRec(string& ori_word, string& rword, vector<vector<string> >& py, vector<pair<string, int> >& rec_word);
	bool wholePinYinRec(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, string& rword, string& ori_word, vector<pair<string, int> >& rec_word);
	bool wholePinYinRec(string& rword, vector<string>& pinyin, string& ori_word, string& fpy, vector<pair<string, int> >& result, vector<pair<string, int> >& rec_word);
	bool firstPinYinRec(string& py, string& rword, vector<pair<string, int> >& rec_word);
	bool chwordRec(vector<vector<string> >& all_py, vector<ushort>& v_code,
			 const string& ori_word, string& rword, vector<pair<string, int> >& rec_word);
	bool enwordRec(vector<string>& vecWord, vector<CHARTYPE>& vecMark, int pos, vector<pair<string, int> >& rec_word);
	bool nearwordRec(const string& word, vector<vector<string> >& whole_pinyin, vector<pair<string, int> >& rec_word);
	void processRec(vector<pair<string, int> >& rec_word, string& ori_word, int& confidence, string& res, bool& state);
	bool recSingleChar(const string& str, vector<CHARTYPE>& vecMark, int cur_pos, string& rec_str);
public:
	Rectify();
	~Rectify();
private:
	NgramModel m_nm;
	CHWordCreator m_chword_create;
	ENWordCreator m_enword_create;
	WholePinYinCreator m_whole_pinyin_create;
private:
	CharacterConverter m_cc;
	WordSegmentation* m_ws;
	map<string, set<string> > m_pinyin_chword;
	map<string, set<string> > m_chword_pinyin;
	map<string, string> m_chword_bishun;
	map<string, vector<string> > m_bishun_chword;
	map<string, set<string> > m_word_cooccur;
	map<ushort, vector<string> > m_word_near;
	static_hash_vector<string, vector<string> > m_word_rword;
	static_hash_map<string, int> m_correct_dict;
	static_hash_vector<string, vector<pair<string, int> > > m_wrong2right;
	static_hash_vector<string, vector<pair<string, int> > > m_wpy, m_fpy;
	session_log_c* m_slog;
	FPTR_WRITE_LOG  m_funcWriteLogptr;//日志函数指针
};

#endif
