#ifndef EDIT_DISTANCE_H
#define EDIT_DISTANCE_H

#include <string>
using namespace std;

struct Node
{
	Node(int ivt_id, int ed, int weight):
	m_ivt_id(ivt_id), m_ed(ed), m_weight(weight)
	{
	}
	bool operator<(const Node& n) const
	{
		return m_ed < n.m_ed || (m_ed == n.m_ed && m_weight > n.m_weight) ||
			(m_ed == n.m_ed && m_weight == n.m_weight && m_ivt_id < n.m_ivt_id);
	}
	int m_ivt_id;
	int m_ed;
	int m_weight;
};

//长度相同，只比较相同位置的字，用于首拼对应的词组
inline int edit_distance_fpy(const string& s1, const string& s2)
{
	if(s1.size() != s2.size())
		return (s1.size() + s2.size()) / 2;
	int ed = 0;
	for(int i = 0; i < s1.size(); i += 2)
	{
		if(*(unsigned short*)&s1[i] != *(unsigned short*)&s2[i])
			++ed;
	}
	return ed;
}

//用于单字节字符串的编辑距离计算
inline int edit_distance(const string& s1, const string& s2)
{
	int i, j, ed = 0;
	int s1_len = s1.size(), s2_len = s2.size();
	int** d = new int*[s1_len+1];
	for(i = 0; i <= s1_len; ++i)
		d[i] = new int[s2_len+1];
	d[0][0] = 0;
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 0; i <= s1_len; ++i)
		d[i][0] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		for(int j = 1; j <= s2_len; ++j)
		{
			d[i][j] = min(min(d[i-1][j] + 1, d[i][j-1] + 1),
				d[i-1][j-1] + (s1[i-1] != s2[j-1]));
		}
	}
	ed = d[s1_len][s2_len];
	for(i = 0; i <= s1_len; ++i)
		delete[] d[i];
	delete[] d;
	return ed;
}

//用于双字节字符串的编辑距离计算
inline int edit_distance_ch(const string& s1, const string& s2)
{
	int i, j, ed = 0;
	int s1_len = s1.size() / 2, s2_len = s2.size() / 2;
	int** d = new int*[s1_len+1];
	for(i = 0; i <= s1_len; ++i)
		d[i] = new int[s2_len+1];
	d[0][0] = 0;
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 0; i <= s1_len; ++i)
		d[i][0] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		for(j = 1; j <= s2_len; ++j)
		{
			d[i][j] = min(min(d[i-1][j] + 1, d[i][j-1] + 1),
				d[i-1][j-1] + (*(short*)&s1[(i-1)*2] != *(short*)&s2[(j-1)*2]));
		}
	}
	ed = d[s1_len][s2_len];
	for(i = 0; i <= s1_len; ++i)
		delete[] d[i];
	delete[] d;
	return ed;
}
#endif
