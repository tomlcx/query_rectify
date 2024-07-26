#ifndef _WORDSEGMENTATION_HPP
#define _WORDSEGMENTATION_HPP

#define MAX_SENTENCE_LEN  1024

#define INFINITY_VALUE	-10000.

/// �������ƥ��ִ��㷨�����ڿ��ٵ� DTRIE �ʵ��㷨
#include "common_headers.h"
#include "dictionary.h"

#include "seg_graph.h"
#include "kshortpath.h"

#include <stack>	
class WordSegmentation
{

public:

	/// �ִ��õĴʵ䡣dictΪ��ʱ���������û�ж���
	//WordSegmentation(  Dictionary *dict){ m_dict = dict;};			
	WordSegmentation(string file)
	{ 
		ifstream ifs(file.c_str());
		string line, yin;
		int pos;
		m_dict = new Dictionary();
		while(getline(ifs, line))
		{
			pos = line.find_first_of("\t");
			yin = line.substr(0, pos);
			m_dict->insert(yin.c_str());
		}
	}

	virtual ~WordSegmentation(void);

	/// �������ƥ��ִ�
	/// pText�Ƿִʵľ���,pData�Ƿִʺ�ʵ�λ�� 
	void word_sect(const char * pText, int * pData);

	/// �������ƥ��ִ�
	/// pText�Ƿִʵľ���,pResult�Ƿִʺ�Ľ��,����\t����
	void word_sect(const char * pText, char * pResult);

	/// ȫ�з�
	/// pText�Ƿִʵľ���,pResult�Ƿִʺ�Ľ��,����\t����
	void word_sect(const char * pText, char * pResult,int a);

	/// �������ƥ����ִ�
	/// pText�Ƿִʵľ���,pResult�Ƿִʺ�Ľ��,����\t����
	void _word_sect(const char * pText, char * pResult);

	bool word_sect_all(const char * pText, vector<vector<string> >& py);
	bool word_sect_all(const char * pText, vector<string>& fpy);

private:
	 
	struct node {
		int _vb;
		int _ve;
		node(int a,int b):_vb(a),_ve(b){
		}
	};
	
	std::vector<int> _tag_text;
	std::vector<node> _node;

	std::vector< std::vector<node> > _path;


private:

	void _get_tag_text(const char * pText) ;

	void _getNode(const char * pText) ;

	const char * _getPath(const char * pText) ;



	Dictionary *m_dict ;	/// �ִʵĴʵ� 
	int  m_nSegCount;

private:

	
	std::stack< node > _stack_path;
	std::stack< node > _stack_buffer;

	std::string _str_all;

public:
	std::string aa(void);
};
#endif /// _WORDSEGMENTATION_HPP
