#ifndef EDIT_DISTANCE_H
#define EDIT_DISTANCE_H

#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

inline char** mallocForEditDistance(int len1, int len2)
{
	int i;
	char** d = new char*[len1+1];
	for(i = 0; i <= len1; ++i)
	{
		d[i] = new char[len2+1];
	}
	return d;
}

inline void freeForEditDistance(int len, char** d)
{
	for(int i = 0; i <= len; ++i)
		delete[] d[i];
	delete[] d;
}

inline char* mallocForEditDistance(int len)
{
	int i;
	char* d = new char[len+1];
	return d;
}

inline void freeForEditDistance(char* d)
{
	delete[] d;
}

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

template<typename T>
inline int edit_distance_sequence(const T& t1, const T& t2)
{
	int min_len = t1.size() > t2.size() ? t2.size() : t1.size();
	int ed = t1.size() > t2.size() ? t1.size() - t2.size() : t2.size() - t1.size();
	for(int i = 0; i < min_len; ++i)
	{
		if(t1[i] != t2[i])
			++ed;
	}
	return ed;
}

//长度相同，只比较相同位置的字
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
	char** d = new char*[s1_len+1];
	for(i = 0; i <= s1_len; ++i)
		d[i] = new char[s2_len+1];
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

inline int edit_distance(const string& s1, const string& s2, char* d)
{
	int i, j, lastdiag, olddiag;
	int s1_len = s1.size(), s2_len = s2.size();
	for(i = 1; i <= s2_len; ++i)
		d[i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		d[0] = i;
		for(j = 1, lastdiag = i - 1; j <= s2_len; ++j)
		{
			olddiag = d[j];
			if(s1[i-1] == s2[j-1])
				d[j] = lastdiag;
			else
				d[j] = min(min((int)d[j], (int)d[j-1]), lastdiag) + 1;
			lastdiag = olddiag;
		}
	}
	return d[s2_len];
}

inline int edit_distance(const string& s1, const string& s2, char** d, int max_ed)
{
	int ok = 1, current = 1, i, j, s, e;
	int s1_len = s1.size(), s2_len = s2.size();
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		ok = 0;
		d[current][0] = i;
		if(i - max_ed >= 1)
			d[current][i-max_ed-1] = 100;
		s = max(i - max_ed, 1);
		e = min(i + max_ed, s2_len);
		for(j = s; j <= e; ++j)
		{
			if(s1[i-1] == s2[j-1])
				d[current][j] = d[1-current][j-1];
			else
				d[current][j] = min(min(d[1-current][j], d[1-current][j-1]), d[current][j-1]) + 1;
			if(d[current][j] <= max_ed)
				ok = 1;
		}
		if(i + max_ed <= s2_len)
			d[current][i+max_ed] = 100;
		if(!ok)
			return 100;
		current = 1 - current;
	}
	return d[1-current][s2_len];
}

inline int edit_distance(char* s1, char* s2, char** d, int max_ed)
{
	int ok = 1, current = 1, i, j, s, e;
	int s1_len = strlen(s1), s2_len = strlen(s2);
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		ok = 0;
		d[current][0] = i;
		if(i - max_ed >= 1)
			d[current][i-max_ed-1] = 100;
		s = max(i - max_ed, 1);
		e = min(i + max_ed, s2_len);
		for(j = s; j <= e; ++j)
		{
			if(s1[i-1] == s2[j-1])
				d[current][j] = d[1-current][j-1];
			else
				d[current][j] = min(min(d[1-current][j], d[1-current][j-1]), d[current][j-1]) + 1;
			if(d[current][j] <= max_ed)
				ok = 1;
		}
		if(i + max_ed <= s2_len)
			d[current][i+max_ed] = 100;
		if(!ok)
			return 100;
		current = 1 - current;
	}
	return d[1-current][s2_len];
}

//用于双字节字符串的编辑距离计算
inline int edit_distance_ch(const string& s1, const string& s2)
{
	int i, j, ed = 0;
	int s1_len = s1.size() / 2, s2_len = s2.size() / 2;
	char** d = new char*[s1_len+1];
	for(i = 0; i <= s1_len; ++i)
		d[i] = new char[s2_len+1];
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

inline int edit_distance_ch(const string& s1, const string& s2, char* d)
{
	int i, j, lastdiag, olddiag;
	int s1_len = s1.size() / 2, s2_len = s2.size() / 2;
	for(i = 1; i <= s2_len; ++i)
		d[i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		d[0] = i;
		for(j = 1, lastdiag = i - 1; j <= s2_len; ++j)
		{
			olddiag = d[j];
			if(*(short*)&s1[(i-1)*2] == *(short*)&s2[(j-1)*2])
				d[j] = lastdiag;
			else
				d[j] = min(min((int)d[j], (int)d[j-1]), lastdiag) + 1;
			lastdiag = olddiag;
		}
	}
	return d[s2_len];
}

inline int edit_distance_ch(const string& s1, const string& s2, char** d, int max_ed)
{
	int ok = 1, current = 1, i, j, s, e;
	int s1_len = s1.size()/2, s2_len = s2.size()/2;
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		ok = 0;
		d[current][0] = i;
		if(i - max_ed >= 1)
			d[current][i-max_ed-1] = 100;
		s = max(i - max_ed, 1);
		e = min(i + max_ed, s2_len);
		for(j = s; j <= e; ++j)
		{
			if(*(short*)&s1[(i-1)*2] == *(short*)&s2[(j-1)*2])
				d[current][j] = d[1-current][j-1];
			else
				d[current][j] = min(min(d[1-current][j], d[1-current][j-1]), d[current][j-1]) + 1;
			if(d[current][j] <= max_ed)
				ok = 1;
		}
		if(i + max_ed <= s2_len)
			d[current][i+max_ed] = 100;
		if(!ok)
			return 100;
		current = 1 - current;
	}
	return d[1-current][s2_len];
}

inline int edit_distance_ch(char* s1, char* s2, char** d, int max_ed)
{
	int ok = 1, current = 1, i, j, s, e;
	int s1_len = strlen(s1)/2, s2_len = strlen(s2)/2;
	for(i = 0; i <= s2_len; ++i)
		d[0][i] = i;
	for(i = 1; i <= s1_len; ++i)
	{
		ok = 0;
		d[current][0] = i;
		if(i - max_ed >= 1)
			d[current][i-max_ed-1] = 100;
		s = max(i - max_ed, 1);
		e = min(i + max_ed, s2_len);
		for(j = s; j <= e; ++j)
		{
			if(*(short*)&s1[(i-1)*2] == *(short*)&s2[(j-1)*2])
				d[current][j] = d[1-current][j-1];
			else
				d[current][j] = min(min(d[1-current][j], d[1-current][j-1]), d[current][j-1]) + 1;
			if(d[current][j] <= max_ed)
				ok = 1;
		}
		if(i + max_ed <= s2_len)
			d[current][i+max_ed] = 100;
		if(!ok)
			return 100;
		current = 1 - current;
	}
	return d[1-current][s2_len];
}

template <typename T>
inline int edit_distance_diff(T t1, T t2)
{
	typename T::iterator it;
	int min_len = t1.size(), max_len = t2.size();
	if(min_len > max_len)
		swap(min_len, max_len);
	sort(t1.begin(), t1.end());
	sort(t2.begin(), t2.end());
	T d(min_len, 0);
	it = set_intersection(t1.begin(), t1.end(), t2.begin(), t2.end(), d.begin());
	return max_len - (it - d.begin());
}
#endif
