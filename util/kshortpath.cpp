
#include "kshortpath.h"


KShortPath::KShortPath(SegGraph iSegGraph)
{
	m_iSegGraph = iSegGraph;
	m_nNumOfVertex = m_iSegGraph.GetNumOfVertex();
	m_pVertex = new VERTEX [m_nNumOfVertex];
}

KShortPath::~KShortPath()
{
	delete [] m_pVertex;	
}


int KShortPath::ShortPath()
{
	//1.
	for(int i=0; i<MAX_SEGMENT_NUM; i++){
		m_pVertex[0].label[i] = 1;
		m_pVertex[0].value[i] = 0;
		m_pVertex[0].pre[i] = -1;
		m_pVertex[0].ks[i] = -1;
	}

	//2.
	for(int ii=1; ii<m_nNumOfVertex; ii++){
		for(int j=0; j<MAX_SEGMENT_NUM; j++){
			m_pVertex[ii].label[j] = 0;
			m_pVertex[ii].value[j] = MAX;
			m_pVertex[ii].pre[j] = -1;
			m_pVertex[ii].ks[j] = -1;
		}
	}
	
	//3.
	int nNumOfEdge = m_iSegGraph.GetNumOfEdge();
	for(int iii=0; iii<nNumOfEdge && m_iSegGraph.m_vecEdgeList[iii].fromVertex == 0; iii++){
		int toVertex = m_iSegGraph.m_vecEdgeList[iii].toVertex;
		m_pVertex[toVertex].value[0] = 0;
		m_pVertex[toVertex].pre[0] = 0;
		m_pVertex[toVertex].ks[0] = 0;
	}
	
	//4.
	while(true){
		if(bTerminate()){ //goto 10, else goto 5.
		//	Output(nResult, nSegCount);
			return 0;
		}	
		else{
			if(GetsPath() == -1)
				break;		
		}
	}
	return 0;
}
bool KShortPath::bTerminate()
{
	int i;
	for(i=0; i<MAX_SEGMENT_NUM; i++){
		if(m_pVertex[m_nNumOfVertex - 1].label[i] != 1)
			break;
	}

	if(i == MAX_SEGMENT_NUM)
		return true;
	return false; 
}

void KShortPath::GetMinValue(int* min, int* vertex_i, int* value_j)
{
	*min = MAX;
	*vertex_i = -1;
	*value_j = -1;
	for(int i=0; i<m_nNumOfVertex; i++){
		for(int j=0; j<MAX_SEGMENT_NUM; j++){
			if(m_pVertex[i].label[j] == 0 && m_pVertex[i].value[j] < *min){
				*min = m_pVertex[i].value[j];
				*vertex_i = i;
				*value_j = j;
			}
		}
	}
} 

int KShortPath::GetsPath()
{
	int min, vertex_i, vertex_k, value_j;
	GetMinValue(&min, &vertex_i, &value_j);
	if (min >= MAX){
		//Output();
		return -1;
	}

	m_pVertex[vertex_i].label[value_j] = 1;

	int nEdgeIndex = m_iSegGraph.m_vecEdgeIndex[vertex_i];

	int nNumOfEdge = m_iSegGraph.GetNumOfEdge();
	for(int i=nEdgeIndex; i<nNumOfEdge && m_iSegGraph.m_vecEdgeList[i].fromVertex==vertex_i; i++){
		for(int p=value_j; p<MAX_SEGMENT_NUM; p++){

			vertex_k = m_iSegGraph.m_vecEdgeList[i].toVertex;

			if(m_pVertex[vertex_k].value[p] > min + 1){
				for(int j=MAX_SEGMENT_NUM-1; j>=p+1; j--){
					m_pVertex[vertex_k].value[j] = m_pVertex[vertex_k].value[j-1];
					m_pVertex[vertex_k].ks[j] = m_pVertex[vertex_k].ks[j-1];
					m_pVertex[vertex_k].pre[j] = m_pVertex[vertex_k].pre[j-1];
				}
				m_pVertex[vertex_k].value[p] = min + 1;
				m_pVertex[vertex_k].pre[p] = vertex_i;
				m_pVertex[vertex_k].ks[p] = value_j;
			
				break;
			}
		}	
	}
	return 0;	
}

int KShortPath::Output(int **nResult, int* nPathCount)
{
	*nPathCount = 0;

	int *path = new int [m_nNumOfVertex];

	memset(path, -1, m_nNumOfVertex*sizeof(int));

	for(int i=0; i<MAX_SEGMENT_NUM; i++){

		if(m_pVertex[m_nNumOfVertex - 1].label[i] != 1){
			break;
		}else{
			int j=0;
			path[j] = m_nNumOfVertex-1;
 
			int ks = i;
			while(path[j] != 0){
				path[j+1]=m_pVertex[path[j]].pre[ks];
				ks = m_pVertex[path[j]].ks[ks];
				j++;
			}
			int k=0;
			while(j>=0){
				nResult[i][k] = path[j];
				j--;
				k++;
			}			
			(*nPathCount)++;
		}
	}
	delete[] path;
	return  1 ;
}

