#ifndef NGRAM_MODEL_H
#define NGRAM_MODEL_H

#include <math.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include "string_process.h"
using namespace std;

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
	NgramModel(const string& in_path, const string& out_path):
	m_in_path(in_path), m_out_path(out_path)
	{
	}
	NgramModel(const string& out_path):m_out_path(out_path)
	{
	}
	bool load(const string& pos_file, const string& prob_file);
	ushort getWordProb(ushort cur_code);
	ushort getShiftProb(ushort cur_code, ushort rcode);
	void findBestPath(vector<vector<ushort> >& words, vector<pair<string, int> >& rec_word);
	int getMinValPos(vector<ushort>& vec, ushort& prob);
	bool create(const string& fname);
	bool save(const string& pos_file, const string& prob_file);
private:
	string m_in_path;
	string m_out_path;
	vector<WordProb> m_chword;
	vector<ShiftProb> m_rchword;
};

#endif
