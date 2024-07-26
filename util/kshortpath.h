#ifndef _KSHORT_PATH_H
#define _KSHORT_PATH_H

#include "seg_graph.h"

#define MAX_SEGMENT_NUM 30
#define MAX	32767

struct Vertex{
	int label[MAX_SEGMENT_NUM];
	int value[MAX_SEGMENT_NUM];
	int pre[MAX_SEGMENT_NUM];
	int ks[MAX_SEGMENT_NUM];
};

typedef struct Vertex VERTEX, *PVERTEX;

class KShortPath{
public:
	KShortPath(SegGraph iSegGraph);
	virtual ~KShortPath();

	int ShortPath();
	bool bTerminate();
	void GetMinValue(int* min, int* vertex_i, int* value_j);
	int GetsPath();
	int Output(int **nResult, int* nSegCount);
		
private:
	SegGraph m_iSegGraph;
	PVERTEX m_pVertex;
	int m_nNumOfVertex;
	bool m_bPathFound;
};

#endif
