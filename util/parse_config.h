#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H
#include <fstream>
#include <map>
#include <vector>
#include "string_process.h"
using namespace std;

inline bool read_config(const string& fname, map<string, string>& config)
{
	ifstream ifs(fname.c_str());
	if(!ifs)
		return false;
	string line;
	vector<string> vec;
	while(getline(ifs, line))
	{
		trim(line);
		if(line == "" || line[0] == '#')
			continue;
		split(line, vec, "=");
		if(vec.size() != 2)
			continue;
		for(int i = 0; i < vec.size(); ++i)
			trim(vec[i]);
		config.insert(make_pair(vec[0], vec[1]));
	}
	return true;
}
#endif
