#include "rectify.h"
#include <sys/time.h>
#include "TimeUtil.h"

int main(int argc, char** argv)
{
	string path = "./";
	Rectify rec;
	rec.init(path);
	vector<pair<string, int> > vec;
	ifstream ifs(argv[1]);
	ofstream ofs(argv[2]);
	string file = "random_";
	file += argv[2];
	ofstream ofs1(file.c_str());
	file += "recall";
	ofstream ofs2(file.c_str());
	string query;
	vector<string> tmp;
	int count = 0, num = 0, linecount = 0, linenum = 0;
	while(getline(ifs, query))
	{
		split(query, tmp, "\t");
		rec.rectify(tmp[0], vec);
		ofs << query;
		if(linecount % 20 == 3 && linenum < 1000)
		{
			ofs2 << query;
			if(vec.size() != 0)
			{
				ofs2 << '\t' << vec[0].first;
			}
			ofs2 << endl;
			++linenum;
		}
		++linecount;
		for(int i = 0; i < vec.size(); ++i)
		{
			ofs << '\t' << vec[i].first;// << '\t' << vec[i].second;
			if(count % 20 == 2 && num < 500)
			{
				++num;
				ofs1 << query << '\t' << vec[i].first << '\t' << vec[i].second << endl;
			}
			++count;
		}
		ofs << endl;
	}
	ofs.close();
	ofs1.close();
	ofs2.close();
}
