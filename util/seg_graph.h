#ifndef _SEGGRAPH_H
#define _SEGGRAPH_H
 
#include <vector>
#include <string.h>
#include "dictionary.h"

#define MAX_WORD_LEN	40
using namespace std;
struct EdgeList{
	int fromVertex;
	int toVertex;
};

typedef struct EdgeList EDGE_LIST;

class SegGraph{
public:
	SegGraph();
	SegGraph(const SegGraph& iSegGraph);
	virtual ~SegGraph();
	
	bool GenerateWordNet(const char* sSentence, Dictionary * dictCore);
	int GetNumOfEdge() { return m_vecEdgeList.size(); }
	int GetNumOfVertex() { return  m_nNumOfVertex; }
	SegGraph& operator=(SegGraph iSegGraph);

	vector<EDGE_LIST> m_vecEdgeList;
	vector<int> m_vecEdgeIndex;
private:
	int m_nNumOfVertex;
};
#endif
