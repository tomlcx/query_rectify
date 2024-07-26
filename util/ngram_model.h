#ifndef NGRAM_MODEL_H
#define NGRAM_MODEL_H

#include <math.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include "string_process.h"
#include "edit_distance.h"
using namespace std;

enum {BEG, END, NUM, ALP};
const char tag[][3] = {"\xA1\x40", "\xA2\x40", "\xA3\xB0", "\xA3\xE1"};
typedef unsigned short ushort;

struct ShiftProb
{
	ShiftProb():code(0), weight(0xffff)
	{
	}
	ushort code;
	ushort weight;
};

struct WordProb
{
	WordProb():start(0), weight(0xffff)
	{
	}
	int start;
	ushort weight;
};

class NgramModel
{
public:
	void init(map<string, string>& config);
	bool load();
	ushort getWordProb(ushort cur_code);
	ushort getShiftProb(ushort cur_code, ushort rcode);
	void findBestPath(vector<vector<ushort> >& words, vector<pair<string, int> >& rec_word);
	int getMinValPos(vector<ushort>& vec, ushort& prob);
	bool create(map<string, int>& words2weight);
	bool save();
private:
	string m_out_path;
	string m_pos_file;
	string m_prob_file;
	vector<WordProb> m_word;
	vector<ShiftProb> m_rword;
};

#endif
