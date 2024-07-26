#ifndef _DICTIONARY_HPP
#define _DICTIONARY_HPP

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
#ifndef _DICTIONARY_Info
#define _DICTIONARY_Info
const  int MIN_CODE = 1;
const  int MAX_CODE = 256;
const  int BC_INC = 1024;
const  int TAIL_INC = 1024;
const	 char POSTFIX_CHAR = char(int(10));
#endif

class Dictionary  
{

public:

	Dictionary(const char *pDict = NULL );

	virtual ~Dictionary();

	// ����ʵ���Ϣ
	int update(const char *ps);

	// ���Һ���
	int search(const char * psWord);

	// ����
	int insert(const char * psWord);

	int fmm(const char * sentence) ;

	// �޶����ȵ��������ƥ�����
	int fmm(const char * sentence,int len) ;
 
	int getWordNum(){return m_nDictWordCount;}

	int bmm(const char * sentence);

	int getMaxLenOfWord()
	{
		return m_nMaxLenOfWord;
	}
private:
	/// 
	void Write_POINT(int n , int point);
	// 
	void READ_TAIL(int p);
	// BASE[n]
	inline int BASE(int n);
	// CHECK[n]
	inline int CHECK(int n);
	// DETAIL[n] mean int ID ; 
	inline int DETAIL(int n);

	// Set BASE[n]  = node 
	void Write_BASE(int n, int node);
	// Set CHECK[n]  = node 
	void Write_CHECK(int n, int node);
	// Set DETAIL[n]  = node 
	void Write_DETAIL(int n, int node);		
	// realloc BASE and CHECK memory , 
	void REALLOC_BC();
	// realloc  tail 
	void REALLOC_TAIL();
	// ��ɲ���ĺ��湤��,�����õ���β��λ�ò��Ҳ��뵽tail��
	void Separate(int s, char * psInsert, int tail_pos,int detail_pos);
	// write into tail   the string is pstemp and the position is p 
	void Write_TAIL(char * psTemp, int p);
	//
	inline int X_CHECK(char *psList);
	// �ı���colision����������
	int CHANGE_BC(int current, int s, char * myList, char ch);
	//TAIL_INSERT
	void TAIL_INSERT(int s, char* a, char* b);
	// BC_INSERT
	void BC_INSERT(int s, char * b);
	// Set_List
	void Set_List(char * myList,int s);	
	// ��ʼ��һ���մʵ䣬׼������
	int InitDict();
	// ��ʼ���ʵ䣬�Ѿ���.trie�ļ�
	int InitDict(const char *ps);
	// ��ʼ���ʵ䣬��û��.trie�ļ�������.trie�ļ�����ʼ��
	int InitDict(const char *ps,int newdict);


private:
	// Base and Check array ,the main Double array, it combine Base and Check array,the array assession is implement by the function Base(n) and Check(n) 
	int* m_pnBC;
	// The tril, it store the discommont prefix(��׺). This is a reduced trie
	char* m_psTAIL;	
	// Temp buffer , using to read the tail
	char m_psTemp[1024];
	// Temp buffer , using to read the tail
	char m_psKey[1024];
	// current maximum position of the double array
	int m_nBC_POS;
	// The current maximum index of tril
	int m_nTAIL_POS;
	// Maximun size of the BASE and CHECK
	int m_nBC_MAX;
	// Maximum size of tail
	int m_nTAIL_MAX;
	// 	��ǰ��������󳤶�;
	int m_nMaxLenOfWord ;

	//����ǰ��ָ��λ��
	int m_nCurrent_Detail_Pos;
	int m_nPreCurrent_Detail_Pos;

	int m_nDictWordCount;
};


#endif //_DICTIONARY_HPP
