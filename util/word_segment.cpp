
#include "word_segment.h" 


WordSegmentation::~WordSegmentation(void)
{

}

void WordSegmentation::word_sect(const char * pText, int * pData)
{
	int nText_Len =(int)strlen(pText);
	int word_pos = 0 ;	
	int fmm = 0 ;
	pData [word_pos] = 0 ;
	while ( pData [word_pos] < nText_Len)
	{	
		fmm =m_dict->fmm( pText + pData[word_pos] ) ;
		if ( fmm == 0 )
		{
			if ( pText[  pData[word_pos] ] > 0 )
			{			
				fmm = 1 ;
			}
			/// 否则,认为是单个的汉字
			else
			{
				fmm = 2 ;
			}		
		}
		pData[word_pos+1] = fmm + pData[word_pos];
		word_pos++;
	}
	return ;
}

void WordSegmentation::word_sect(const char * pText, char * pResult)
{
	int word_start = 0 ; 
	int word_len = 0 ; 
	int nrPos = 0 ;
	while ( pText[ word_start ] != '\0')
	{	 
			/// 在词典中查找最大匹配的长度
			word_len =m_dict->fmm( pText + word_start) ;
			if ( word_len == 0 )/// 不在词典中出现
			{
				/// 如果是ascii吗,则分析
				if ( pText[ word_start ] > 0)
				{			
					word_len = 1 ;
				}
				/// 否则,认为是单个的汉字
				else
				{
					word_len = 2 ;
				}
			}
			strncpy(pResult+nrPos,pText + word_start,word_len) ;	
			nrPos += word_len ;
			pResult[nrPos++] = '\t';	
			word_start = word_len + word_start; 
	}	
	pResult[nrPos] = '\0';
	return ; 
}

void WordSegmentation::_word_sect(const char * pText, char * pResult)
{
	std::vector< std::string > buffer ; 
	int word_pos = 0 ; 
	int nrPos = strlen(pText) ;
	strcpy(pResult,pText);
	int pos = 0 ;
	while ( nrPos > 0 )
	{	 
		/// 在词典中查找最大匹配的长度
		word_pos =m_dict->bmm( pResult ) ;
		if ( word_pos == -1 )/// 不在词典中出现
		{
			word_pos = nrPos - 1 ;			
		}
		buffer.push_back(pResult + word_pos) ;
		nrPos = word_pos ;
		pResult[nrPos] = '\0';	 
	}
	std::string str ; 	
	for ( int i = buffer.size() - 1  ; i >= 0 ; i-- )
	{
		str += buffer[i];
		str += "\t";
	}
	strcpy(pResult,str.c_str());
	return ; 
}

void WordSegmentation::word_sect(const char * pText, char * pResult,int a)
{
	std::string str ;

	// 正向最大匹配的结果
	word_sect(pText, pResult);
	str += pResult ;
	str += "\n";

	// 逆向最大匹配的结果
	_word_sect(pText, pResult);
	str += pResult ;
	str += "\n";

	strcpy(pResult,str.c_str());
	return ; 
}

void WordSegmentation::_get_tag_text(const char * pText)
{
	int len = strlen(pText);

	_tag_text.clear();
	for ( int i = 0 ; i < len ; i++)
	{
		_tag_text.push_back(i);
	}
}

void WordSegmentation::_getNode(const char * pText)
{ 
	int len = strlen(pText);
	_node.clear();
	char tmp[1024];
 	for ( int i = 0 ; i <= len ; i++)
	{
		for ( int j = i+1 ; j <= len ; j++)
		{
			strncpy(tmp,pText+i,j-i);
		 
			tmp[j-i] = '\0';

			if ( m_dict->search(tmp) > 0)
			{
				_node.push_back(node(i,j));
			}		 
		}
	}
}

const char * WordSegmentation::_getPath(const char * pText)
{
	//_str_all.clear();
	 _str_all.erase(_str_all.begin(),_str_all.end());

	//std::cout<<pText<<"\n";
	 printf("%s\n",pText);
	char ctmp[128];
	for ( size_t i = 0 ; i < _node.size() ; i++)
	{
//		std::cout<<_node[i]._vb<<"\t"<<_node[i]._ve<<"\n";
	}

	for (size_t j = 0 ; j < _node.size() ; j++)
	{
		if ( _node[j]._vb == 0)
		{
			_stack_buffer.push(_node[j]);
		}
	}
	int end = strlen(pText);
	while ( !_stack_buffer.empty())
	{
		node node_val = _stack_buffer.top();	
		_stack_buffer.pop();

		while ( !_stack_path.empty() && _stack_path.top()._ve >= node_val._ve )
		{
			_stack_path.pop();
		}
		_stack_path.push(node_val);

		// 到达了终点
		if ( node_val._ve == end )
		{
			std::stack<node> tmp;
			while ( !_stack_path.empty())
			{
				tmp.push(_stack_path.top());
				_stack_path.pop();
			}
			std::string ss ;
			while ( !tmp.empty())
			{
				node val = tmp.top();

				strncpy(ctmp,pText+val._vb,val._ve - val._vb);

				ctmp[val._ve - val._vb] = '\0';	

				_str_all += ctmp; 
				_str_all += "\t"; 
				ss += ctmp ;
				ss +="\t";
			//	std::cout<<"("<<val._vb<<"\t"<<val._ve<<")\t";
				_stack_path.push(val);
				tmp.pop();
			}
			_stack_path.pop();
		 	//std::cout<<ss<<"\n";
			printf("%s\n",ss.c_str());
			_str_all += "\n"; 
			continue;
		}
		bool hasmore = false ;
		for ( size_t k = 0 ; k < _node.size() ; k++ )
		{
			if ( _node[k]._vb == node_val._ve  )
			{
				node v = _node[k];
				_stack_buffer.push(_node[k]);
				hasmore = true ;
			}
		}

		if ( !hasmore )
		{ 
			_stack_path.pop(); 
		}


	}
	while ( !_stack_path.empty())
	{
		_stack_path.pop();
	}
	return _str_all.c_str();

}

bool WordSegmentation::word_sect_all(const char * pText, vector<vector<string> >& py)
{ 
	py.clear();
	int **nSegRoute;
	nSegRoute = new int*[MAX_SEGMENT_NUM];
	for(int i=0; i<MAX_SEGMENT_NUM; i++){
		nSegRoute[i] = new int [MAX_SENTENCE_LEN/2];
		memset(nSegRoute[i], -1, MAX_SENTENCE_LEN/2*sizeof(int));
	}

	SegGraph iSegGraph;
	iSegGraph.GenerateWordNet(pText, m_dict);

	KShortPath iKShortPath(iSegGraph);
	iKShortPath.ShortPath();
	iKShortPath.Output(nSegRoute, &m_nSegCount);

#if 0
	for ( int ii = 0 ; ii < MAX_SEGMENT_NUM ; ii++)
	{
		for ( int jj = 0 ; jj < MAX_SENTENCE_LEN/2 ; jj++)
		{
			if ( nSegRoute[ii][jj] == -1)
			{
				break ;
			}
			std::cout<< nSegRoute[ii][jj]<<"\t";
		}
		std::cout<<"\n";
	}
#endif	

	std::string str ;
	char ctmp[128];
	int pos = 0;
	bool havnew = false;
	for ( int iii = 0 ; iii < MAX_SEGMENT_NUM ; iii++)
	{
		int pre = 0 ;
		havnew = false;
		for ( int jjj = 1 ; jjj < MAX_SENTENCE_LEN/2 ; jjj++)
		{
			if ( nSegRoute[iii][jjj] == -1)
			{
				break ;
			}
			strncpy(ctmp,pText+pre,nSegRoute[iii][jjj] - pre);

			ctmp[nSegRoute[iii][jjj] - pre] = '\0';	

			pre = nSegRoute[iii][jjj];

			//str += ctmp; 
			//str += "\t";   
			py.resize(pos + 1);
			py[pos].push_back(ctmp);
			havnew = true;
		}
		if(havnew)
			++pos;
		//str += "\n"; 
	}

	//strncpy(pResult,str.c_str(),buffer_len-1);
	//pResult[buffer_len-1]=0;

	for(int iiii=0; iiii<MAX_SEGMENT_NUM; iiii++){
		delete []nSegRoute[iiii] ;
	}
	delete []nSegRoute;
	return py.size() > 0;

}


bool WordSegmentation::word_sect_all(const char * pText, vector<string>& fpy)
{ 
	fpy.clear();
	int **nSegRoute;
	nSegRoute = new int*[MAX_SEGMENT_NUM];
	for(int i=0; i<MAX_SEGMENT_NUM; i++){
		nSegRoute[i] = new int [MAX_SENTENCE_LEN/2];
		memset(nSegRoute[i], -1, MAX_SENTENCE_LEN/2*sizeof(int));
	}

	SegGraph iSegGraph;
	iSegGraph.GenerateWordNet(pText, m_dict);

	KShortPath iKShortPath(iSegGraph);
	iKShortPath.ShortPath();
	iKShortPath.Output(nSegRoute, &m_nSegCount);

#if 0
	for ( int ii = 0 ; ii < MAX_SEGMENT_NUM ; ii++)
	{
		for ( int jj = 0 ; jj < MAX_SENTENCE_LEN/2 ; jj++)
		{
			if ( nSegRoute[ii][jj] == -1)
			{
				break ;
			}
			std::cout<< nSegRoute[ii][jj]<<"\t";
		}
		std::cout<<"\n";
	}
#endif	

	std::string str ;
	char ctmp[128];
	int pos = 0;
	bool havnew = false;
	for ( int iii = 0 ; iii < MAX_SEGMENT_NUM ; iii++)
	{
		int pre = 0 ;
		havnew = false;
		for ( int jjj = 1 ; jjj < MAX_SENTENCE_LEN/2 ; jjj++)
		{
			if ( nSegRoute[iii][jjj] == -1)
			{
				break ;
			}
			strncpy(ctmp,pText+pre,nSegRoute[iii][jjj] - pre);

			ctmp[nSegRoute[iii][jjj] - pre] = '\0';	

			pre = nSegRoute[iii][jjj];

			//str += ctmp; 
			//str += "\t";   
			fpy.resize(pos + 1);
			fpy[pos].push_back(ctmp[0]);
			havnew = true;
		}
		if(havnew)
			++pos;
		//str += "\n"; 
	}

	//strncpy(pResult,str.c_str(),buffer_len-1);
	//pResult[buffer_len-1]=0;

	for(int iiii=0; iiii<MAX_SEGMENT_NUM; iiii++){
		delete []nSegRoute[iiii] ;
	}
	delete []nSegRoute;
	return fpy.size() > 0;

}

std::string WordSegmentation::aa(void)
{
	return std::string();
}
