// DoubleArrayTrieDictionary.cpp: implementation of the DoubleArrayTrieDictionary class.
//
//////////////////////////////////////////////////////////////////////

#include "dictionary.h" 
#include <stdio.h>

Dictionary::Dictionary(const char *pDict)
: m_pnBC(NULL)
, m_psTAIL(NULL)  
, m_nBC_POS(0)   
, m_nTAIL_POS(0)
, m_nBC_MAX(0)
, m_nTAIL_MAX(0)
, m_nMaxLenOfWord(0)
, m_nPreCurrent_Detail_Pos(0)
, m_nDictWordCount(0)

{ 
	if ( pDict == NULL)
	{
		InitDict();
	}
	else
	{
		std::string str = pDict ;
		std::string strtrie = str.substr(0,str.rfind('.')) + ".trieex";
		FILE *fp = fopen(strtrie.c_str(),"rb") ;
		if (fp != NULL)
		{
			InitDict(strtrie.c_str());
		}
		else
		{
			InitDict(pDict,1);
		}
		
	}
}

Dictionary::~Dictionary(void)
{
	if ( m_pnBC != NULL)
	{
		free(m_pnBC);
		m_pnBC = NULL;
	}
	if ( m_psTAIL != NULL)
	{
		free(m_psTAIL);
		m_psTAIL = NULL;
	}
}

// BASE[n]
int Dictionary::BASE(int n)
{
	if ( n > m_nBC_POS )
		return 0;
	else return (m_pnBC[3*n]);
}

// CHECK[n]
int Dictionary::CHECK(int n)
{
	if ( n > m_nBC_POS )
		return 0;
	else return (m_pnBC[3*n+1]);
}

// Set BASE[n]  = node 
void Dictionary::Write_BASE(int n, int node)
{
	while ( n >= m_nBC_MAX)
		REALLOC_BC();	
	if( n > m_nBC_POS ) m_nBC_POS = n ;
	m_pnBC[3*n] =  node ;
}
// DETAIL[n]
int Dictionary::DETAIL(int n)
{
	if ( n > m_nBC_POS )
		return 0;
	else return (m_pnBC[3*n+2]);
}

// Set CHECK[n]  = node 
void Dictionary::Write_CHECK(int n, int node)
{
	while ( n >= m_nBC_MAX)
		REALLOC_BC();	
	if( n > m_nBC_POS ) m_nBC_POS = n ;
	m_pnBC[3*n + 1] = node ;
}

// Set DETAIL[n]  = node 
void Dictionary::Write_DETAIL(int n, int node)
{
	while ( n >= m_nBC_MAX)
		REALLOC_BC();	
	if( n > m_nBC_POS ) m_nBC_POS = n ;
	m_pnBC[3*n+2] = node ;
}
// write into tail   the string is pstemp and the position is p 
void Dictionary::Write_TAIL(char * psTemp, int p)
{
	int tail_index = p ;
	while ( (int)( p+strlen(psTemp) ) >= m_nTAIL_MAX -1 )
		REALLOC_TAIL();
	//拷贝到tail中
	int i = 0 ;
	while ( *(psTemp+i) != '\0' )
		m_psTAIL[tail_index++] = *(psTemp +i++);
	if ( ( p + i + 1 ) > m_nTAIL_POS ) m_nTAIL_POS = p + i ;
	return ;
	
}

// realloc BASE and CHECK memory , 
void Dictionary::REALLOC_BC(void)
{
	int pre_bc = m_nBC_MAX  ;
	m_nBC_MAX += BC_INC ;
	if ( ( m_pnBC = (int *)realloc(m_pnBC,sizeof(int)*3*m_nBC_MAX ) ) == NULL )
	{
		//std::cout<<"BC:无法分配内存\n";
		printf("BC:无法分配内存\n");
		exit(-1);
	}
	for (int i = pre_bc*3 ; i < 3*m_nBC_MAX; i++)
		m_pnBC[i] = 0 ; 
	return ;   
}

// realloc  tail 
void Dictionary::REALLOC_TAIL(void)
{
	int pre_tail = m_nTAIL_MAX ;
	m_nTAIL_MAX += TAIL_INC ;
	if ( ( m_psTAIL = (char *)realloc(m_psTAIL,sizeof(char)*m_nTAIL_MAX ) ) == NULL )
	{
		//std::cout<<"TAIL:无法分配内存\n";
		printf("TAIL:无法分配内存\n");
		exit(-1);
	}
	for(int i = pre_tail; i < m_nTAIL_MAX ; i++) m_psTAIL[i] = '\0';
	return ;
}

// 完成插入的后面工作,在设置单词尾的位置并且插入到tail中
void Dictionary::Separate(int s, char * psInsert, int tail_pos,int detail_pos)
{
	int t = BASE(s) + (unsigned char )*psInsert ;
	Write_CHECK(t,s) ;
	Write_BASE(t,(-1)*tail_pos); 
	Write_DETAIL(t,detail_pos);
 	psInsert++; //指针下移一位,插入到tail中
	Write_TAIL(psInsert,tail_pos);
	return ;
	
}

//
int Dictionary::X_CHECK(char *psList)
{
	int i = 0 ;
	int base_pos = 1 ;
	do {
		unsigned char ch = psList[i++];
		int check_pos = base_pos + ch ;
		//找到同时符合 psList里每一个字符条件的最小的check_pos
		if ( CHECK(check_pos) != 0 )
		{
			base_pos++;			
			i=0;
			continue;
		}
	} while(psList[i] != '\0');
	return base_pos;
}

// 改变由colision产生的问题
int Dictionary::CHANGE_BC(int current, int s, char * mylist, char ch)
{
	char a_list[MAX_CODE - MIN_CODE];
	int old_base = BASE(s);
	size_t i = 0 ;
   	if (ch != '\0')
	{
		strcpy(a_list,mylist);
		i = strlen(a_list);	
 		a_list[i]=ch;
 		a_list[i+1]='\0';
	}else{
		strcpy(a_list,mylist);
		i = strlen(a_list);
	}

	Write_BASE( s , X_CHECK(a_list) ) ;
	i = 0 ;	
	do{
		int old_node = old_base + (unsigned char) (*mylist);
		int new_node = BASE(s) + (unsigned char) (*mylist);
		Write_BASE(new_node, BASE(old_node)); 
		Write_DETAIL(new_node,DETAIL(old_node));		
		Write_CHECK(new_node, s) ;
		if( BASE(old_node) > 0 ) 
		{
					int k = BASE(old_node)+1;
			while (k-BASE(old_node) < MAX_CODE-MIN_CODE || k < m_nBC_POS) 
			{
				if(CHECK(k) == old_node)
					Write_CHECK(k, new_node);
				++k ;
			}
		}
		if(current != s && old_node == current) 
			current = new_node;
		Write_BASE(old_node, 0);  
		Write_DETAIL(old_node, 0); 		
		Write_CHECK(old_node, 0) ; 
		mylist++;
	}while(*mylist != '\0'); 
	return(current); 
}

void Dictionary::TAIL_INSERT(int s, char* a, char* b)
{
	char mylist[3]; 	 
	int old_tail_pos = (-1) *BASE(s); 
	int old_detail_pos = DETAIL(s) ;	
	int length = 0 ;
	while (a[length] == b [length] ) length++;
	int i = 0; 
	while (i < length)
	{
		unsigned char ch = a[i++];
		mylist [0] = ch; mylist [1] = '\0';
		Write_BASE(s, X_CHECK(mylist));
		int t = (  BASE(s) + ch );
		Write_CHECK (t, s);		
		s=t;
	}
	mylist[0] = a[length];
	mylist[1] = b[length]; 
	mylist[2] = '\0';
	Write_BASE(s,X_CHECK(mylist));
	Separate(s,a+length, old_tail_pos , old_detail_pos);
	Separate(s, b+length, m_nTAIL_POS, m_nPreCurrent_Detail_Pos );
}

// BC_INSERT
void Dictionary::BC_INSERT(int s, char * b)
{ 
	char list_s[MAX_CODE-MIN_CODE+1], list_t[MAX_CODE-MIN_CODE+1 ];
	int t = BASE(s) + (unsigned char)*b; 
	if(CHECK(t) != 0)
	{		
		Set_List(list_s,s);
		Set_List(list_t, CHECK(t));
		if(strlen(list_s) +1 < strlen(list_t))
			s = CHANGE_BC(s, s, list_s, *b);
		else s = CHANGE_BC(s, CHECK(t), list_t, '\0');
	}
	Separate(s, b, m_nTAIL_POS,m_nPreCurrent_Detail_Pos);
}

// Set_List    myList 必须是char[MAX_CODE-MIN_CODE+1]的数组
void Dictionary::Set_List(char * myList,int s)
{
	int j = 0;
	for (int i = MIN_CODE; i < MAX_CODE-1; i++) 
	{
		int t = BASE(s)+i;
		if (CHECK(t) == s)
			myList[j++] = (unsigned char)i;
	} 
	myList[j] = '\0';
//	if (strlen(myList) == 0 )
//		std::cout<<"";
}

// 查找函数
int Dictionary::search(const char * psWord)
{ 
	strcpy(m_psKey,psWord); 
	if (psWord[0] == POSTFIX_CHAR && psWord[1] == '\0' )return -1 ;
	size_t wordlen = strlen(m_psKey) ;
	m_psKey[wordlen] = POSTFIX_CHAR ;
	m_psKey[wordlen+1] = '\0' ;	
//	strcat (m_psKey, "?");
	int h = -1 ;
	int  s=1, t;
	int point = -1 ;
	do 
	{
		++h ;
		unsigned char ch = (unsigned char)m_psKey[h];
		t = BASE(s) + ch;	
		if (CHECK(t) != s) 
		{
			return -1 ;
		}
		if(BASE(t) < 0){ 
			point = DETAIL(t);
			break;
		}
		s=t;
	}while(true);
	if(*(m_psKey+h) != POSTFIX_CHAR) READ_TAIL ((-1) *BASE(t)) ; 
	if( *(m_psKey+h) == POSTFIX_CHAR)		
	{  
		return point;	
	}
	if( *(m_psKey+h) == POSTFIX_CHAR|| 0== strcmp(m_psTemp, (m_psKey+h+1)))		
	{  
		return point;	
	}
	if ( 0 != strcmp(m_psTemp, (m_psKey+h+1))  )
	{
		return -1 ; 
	} 
	return point;
}

// 插入
int Dictionary::insert(const char * psWord)
{

	strcpy(m_psKey,psWord); 
	if (psWord[0] == POSTFIX_CHAR && psWord[1] == '\0' )return -1 ;
	int wordlen = (int)strlen(m_psKey) ;
	m_psKey[wordlen] = POSTFIX_CHAR ;
	m_psKey[wordlen+1] = '\0' ;	
//	strcat (m_psKey, "?");
	int h = -1 ;
	int  s=1, t;
	do 
	{
		++h ;
		unsigned char ch = (unsigned char)m_psKey[h];
		t = BASE(s) + ch;	 
		if (CHECK(t) != s) 
		{
			m_nPreCurrent_Detail_Pos++ ;
			BC_INSERT(s, m_psKey+h) ;
			m_nDictWordCount++;
			m_nMaxLenOfWord = (wordlen > m_nMaxLenOfWord ) ? wordlen:m_nMaxLenOfWord;
			return(1);
		}
		if(BASE(t) < 0) break;
		s=t;
	}while(true);
	if(*(m_psKey+h) != POSTFIX_CHAR ) READ_TAIL ((-1) *BASE(t)) ; 
	if(*(m_psKey+h) == POSTFIX_CHAR || !strcmp(m_psTemp, (m_psKey+h+1))) 
	{
		return(-1);
	}
	else
	{		
 		if (BASE(t) != 0)
		{	
			m_nPreCurrent_Detail_Pos ++ ;
			TAIL_INSERT(t, m_psTemp, m_psKey+h+1);
			m_nDictWordCount++;
			m_nMaxLenOfWord = (wordlen > m_nMaxLenOfWord ) ? wordlen:m_nMaxLenOfWord;
			return(1);
		} 
	}
	return -1;
}

// 初始化
int Dictionary::InitDict(void)
{
	m_nBC_MAX = BC_INC; m_nBC_POS = 1; m_nTAIL_POS = 1;
	if( (m_pnBC = (int *)malloc (sizeof (int) *3*m_nBC_MAX) ) == NULL) 
	{
		//std::cout<<"BC malloc error!!\n";
		printf("BC malloc error!!\n");
		exit(-1);
	}
	memset(m_pnBC, 0,sizeof(int)*3*m_nBC_MAX);
	Write_BASE(1,1);
	m_nTAIL_MAX = TAIL_INC ;
	if( (m_psTAIL = (char *)malloc (sizeof (char) *m_nTAIL_MAX) ) == NULL) 
	{
		//std::cout<<"TAIL malloc error!!\n";
		printf("TAIL malloc error!!\n");
		exit(-1);
	} 
	m_nTAIL_POS = 1; m_psTAIL[0]=POSTFIX_CHAR;
	return 0; 
}


void Dictionary::READ_TAIL(int p)
{
	int i = 0 ;
	while ( (m_psTAIL[p]) != POSTFIX_CHAR )
	{
		m_psTemp[i++] = m_psTAIL [p++] ;
	
	}
	m_psTemp[i++] = POSTFIX_CHAR;
	m_psTemp[i] = '\0';
}

int Dictionary::update(const char *ps)
{
	FILE *stream  = fopen(ps,"wb");
	if ( stream != NULL ){
		fwrite(&m_nBC_MAX,sizeof(int),1,stream);
		fwrite(&m_nBC_POS,sizeof(int),1,stream);
		fwrite(m_pnBC,sizeof(int),m_nBC_MAX*3,stream);
		fwrite(&m_nTAIL_MAX,sizeof(int),1,stream);
		fwrite(&m_nTAIL_POS,sizeof(int),1,stream);
		fwrite(m_psTAIL,sizeof(char),m_nTAIL_MAX,stream);
		fwrite(&m_nMaxLenOfWord,sizeof(int),1,stream);
		fwrite(&m_nDictWordCount,sizeof(int),1,stream);
		fclose(stream);
		return 1;
	}
	return 0;
}

int Dictionary::InitDict(const char *ps)
{
	FILE *stream  = fopen(ps,"rb");
	if ( stream != NULL ){
		fread(&m_nBC_MAX,sizeof(int),1,stream);
		if( (m_pnBC = (int *)malloc (sizeof (int) *3*m_nBC_MAX) ) == NULL) 
		{
			//std::cout<<"BC malloc error!!\n";
			printf("BC malloc error!!\n");
			exit(-1);
		} 
		fread(&m_nBC_POS,sizeof(int),1,stream);
		fread(m_pnBC,sizeof(int),m_nBC_MAX*3,stream);
		fread(&m_nTAIL_MAX,sizeof(int),1,stream);
		if( (m_psTAIL = (char *)malloc (sizeof (char) *m_nTAIL_MAX) ) == NULL) 
		{
			//std::cout<<"TAIL malloc error!!\n";
			printf("TAIL malloc error!!\n");
			exit(-1);
		}
		fread(&m_nTAIL_POS,sizeof(int),1,stream);
		fread(m_psTAIL,sizeof(char),m_nTAIL_MAX,stream);

		fread(&m_nMaxLenOfWord,sizeof(int),1,stream);
		fread(&m_nDictWordCount,sizeof(int),1,stream);
		m_nPreCurrent_Detail_Pos = m_nDictWordCount; // 设置当前的ID
		fclose(stream);
		return 1;
	}
	return 0;
}

int Dictionary::InitDict(const char *ps,int newdict)
{ 
	newdict = 1 ;
	InitDict();
	std::ifstream in(ps);  
//	std::cout<<"Init trie dict!may take some time!!\n";
	if (!in.is_open())
	{
		return -1;
	}
	int num = 0 ; 
	while (!in.eof())
	{
		std::string str;
		in>>str; 
		insert(str.c_str()); 
		if(num++ % 100 == 0)
		{
			//std::cout<<"Insert word:\t"<<num<<"\t\t\r";
			printf("Insert word:\t %d\t\t\r",num);
		}
		
	}
	in.close();
	std::string str = ps ;
	std::string strout = str.substr(0,str.rfind('.')) + ".trieex";
	update(strout.c_str());
	return 1 ;
}

//正向最大匹配查找双trie
int Dictionary::fmm(const char *sentence) 
{
	int val = 0 ;
	int CurMax  = 0 ;
	strncpy(m_psKey,sentence,m_nMaxLenOfWord);

	m_psKey[m_nMaxLenOfWord] = POSTFIX_CHAR;
	m_psKey[m_nMaxLenOfWord+1] = '\0';		

//	strcat (m_psKey, "?");
	int h = -1 ;
	int  s=1, t;
	unsigned char ch_end = POSTFIX_CHAR;
	do 
	{
		++h ;
		unsigned char ch = (unsigned char)m_psKey[h];
		t = BASE(s) + ch;	
		int c = BASE(s) + int(ch_end) ;	
		
		if(BASE(c) < 0 && CHECK(c) == s )  
		{
			CurMax = val ;
			if(ch == POSTFIX_CHAR)break;
		}
		val++;
		if (CHECK(t) != s) 
		{
			break ;
		}
		if(BASE(t) < 0)
		{ 
			if(*(m_psKey+h) != POSTFIX_CHAR)
			{ 
				READ_TAIL ((-1) *BASE(t)) ; 
				int i = 0 ;
				while ( m_psTemp[i] == m_psKey[++h] && m_psTemp[i] != POSTFIX_CHAR )
				{
					i++;
					val++;
				}
				if (m_psTemp[i] == POSTFIX_CHAR)
				{
					CurMax = val ;
				} 
			}
			break;
		}
		s=t;
	}while(true); 
	return CurMax;
}


int Dictionary::fmm(const char *sentence,int len) 
{
	int val = 0 ;
	int CurMax  = 0 ;
	strncpy(m_psKey,sentence,len);
	m_psKey[len] = POSTFIX_CHAR;
	m_psKey[len+1] = '\0';		

//	strcat (m_psKey, "?");
	int h = -1 ;
	int  s=1, t;
	unsigned char ch_end = POSTFIX_CHAR;
	do 
	{
		++h ;
		unsigned char ch = (unsigned char)m_psKey[h];
		t = BASE(s) + ch;	
		int c = BASE(s) + int(ch_end) ;	
		
		if(BASE(c) < 0 && CHECK(c) == s )  
		{
			CurMax = val ;
			if(ch == POSTFIX_CHAR)break;
		}
		val++;
		if (CHECK(t) != s) 
		{
			break ;
		}
		if(BASE(t) < 0)
		{ 
			if(*(m_psKey+h) != POSTFIX_CHAR)
			{ 
				READ_TAIL ((-1) *BASE(t)) ; 
				int i = 0 ;
				while ( m_psTemp[i] == m_psKey[++h] && m_psTemp[i] != POSTFIX_CHAR )
				{
					i++;
					val++;
				}
				if (m_psTemp[i] == POSTFIX_CHAR)
				{
					CurMax = val ;
				} 
			}
			break;
		}
		s=t;
	}while(true); 
	return CurMax;
}
int Dictionary::bmm(const char *sentence) 
{
	int pos = 0 ;

	int len = (int)strlen(sentence) ;

	if ( len > m_nMaxLenOfWord )
	{
		pos  =  len - m_nMaxLenOfWord ;
	}

	const char *p = sentence + pos ;
	for ( int i = pos ; i < len ; i++ )
	{
		if ( search( sentence + i ) > 0)
		{
			return i ;
		}
	}
	return -1 ;
}
