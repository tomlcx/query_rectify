#include <iostream>
#include <ext/hash_map>
#include <vector>
#include <fstream>
#include "hash_wrap.h"
#include "string_process.h"
#include "static_hash_vector.h"
using namespace std;
using namespace __gnu_cxx;

int main(int argc, char** argv)
{
	string str1 = argv[1];
	string sav_path1 = "data/" + str1;
	ifstream ifs((str1 + ".txt").c_str());
	string line;
	int count = 0;
	vector<pair<string, int> > rs;
	while(getline(ifs, line))
	{
		count = count + 1;
		rs.push_back(make_pair(line, count));
	}
	ifs.close();
	ifs.clear();
	static_hash_map<string, int> shm;
	int bucket = (int)(ceil(log((double)count)/log(2.0)));
	shm.container_to_hash_file(rs, bucket, sav_path1.c_str());
	cout << argv[1] << " size:" << count << endl;

	string str2 = argv[2];
	string sav_path2 = "data/" + str2;
	ifs.open((str2 + ".txt").c_str());
	count = 0;
	vector<string> vec;
	vector<string> tmp;
	hash_map<string, vector<pair<string, int> > > word2right2weight;
	while(getline(ifs, line))
	{
		split(line, vec, "\t");
		if(vec.size() != 2)
		{
			cout << "vec size not equal 2" << endl;
			continue;
		}
		split(vec[1], tmp, "|");
		if(vec[0].size() >= STRMAXNUM || tmp[0].size() >= STRMAXNUM)
		{
			cout << "out of max len:" << vec[0] << " or " << tmp[0] << endl;
			continue;
		}
		count = count + 1;
		word2right2weight[vec[0]].push_back(make_pair(tmp[0], atoi(tmp[1].c_str())));
	}
	bucket = (int)(ceil(log((double)count)/log(2.0)));
	static_hash_vector<string, vector<pair<string, int> > > shv;
	shv.write(word2right2weight, sav_path2);
	cout << "rec data size:" << count << endl;
}
