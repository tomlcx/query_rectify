
#include"seg_graph.h"
SegGraph::SegGraph()
{
}

SegGraph::SegGraph(const SegGraph& iSegGraph)
{
	m_vecEdgeList = iSegGraph.m_vecEdgeList;
	m_vecEdgeIndex = iSegGraph.m_vecEdgeIndex;	
	m_nNumOfVertex = iSegGraph.m_nNumOfVertex;
}
SegGraph::~SegGraph()
{
}

bool SegGraph::GenerateWordNet(const char* sSentence, Dictionary* dictCore)
{
	m_vecEdgeList.clear();
	m_vecEdgeIndex.clear();

	m_nNumOfVertex = strlen(sSentence) + 1;
	int nMaxLenOfWord = dictCore->getMaxLenOfWord();

	for(int i=0; i<m_nNumOfVertex; i++){
		int nEdgeNum = m_vecEdgeList.size();
		m_vecEdgeIndex.push_back(nEdgeNum);
		char sKey[MAX_WORD_LEN];
		strncpy(sKey, sSentence+i, nMaxLenOfWord);
		sKey[nMaxLenOfWord] = '\0';

		int CurMax = strlen(sKey);
		while(CurMax > 0){
			if( dictCore->search(sKey) > 0)
			{
				//an edge was found, store it in the edge list
				EDGE_LIST sEdgeList;
				sEdgeList.fromVertex = i;
				sEdgeList.toVertex = i + CurMax;
				m_vecEdgeList.push_back(sEdgeList);
			}
			sKey[--CurMax] = '\0';
		}
	}	

/*#ifdef _DEBUG
	printf("\n\nEdges are:\n");
	for(int ii=0;ii<m_vecEdgeList.size(); ii++){
		printf("(%d %d) ", m_vecEdgeList[ii].fromVertex, m_vecEdgeList[ii].toVertex);
	}
	printf("\n");


	printf("\nEdges index is:\n");
	for(int iii=0; iii<m_vecEdgeIndex.size(); iii++){
		printf("%d ", m_vecEdgeIndex[iii]);
	}
	printf("\n");
#endif*/

	return true;
}

SegGraph& SegGraph::operator=(SegGraph iSegGraph)
{
	m_vecEdgeList = iSegGraph.m_vecEdgeList;
	m_vecEdgeIndex = iSegGraph.m_vecEdgeIndex;	
	m_nNumOfVertex = iSegGraph.m_nNumOfVertex;
	return *this;
}
