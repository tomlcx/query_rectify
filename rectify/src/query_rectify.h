#ifndef QUERY_RECTIFY_H
#define QUERY_RECTIFY_H

#include <vector>
#include <string>
using namespace std;

class session_log_c;

typedef void (*FPTR_WRITE_LOG)(void* pLogger, bool bSingle, int level, const char* file, int line, const char *fmt, ...);

class QueryRectify
{
public:
	virtual bool rectify(const string& query, vector<pair<string, int> >& rec)=0;
	virtual bool init(const string& path)=0;
	virtual bool initSpecial(session_log_c* m_log, FPTR_WRITE_LOG m_logFunc)=0;
	static QueryRectify* create();
	virtual ~QueryRectify(){};
};

#endif
