#ifndef _STATIC_HASH_VECTOR_
#define _STATIC_HASH_VECTOR_

#include "nokey_static_hash.h"
#include <ext/hash_map>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace __gnu_cxx;
#define STRMAXNUM  25
typedef unsigned long long u64;
enum T2Type{fixed_hash,vector_fixed,vector_string,vector_pair_string_int,T2_others};

typedef struct HashVector
{
	char* data ;     //数据区地址
	int count ;         //数据个数
	int size ;          //每个数据类型的大小
}HASHVECTOR;

typedef struct PriceRange
{
	int min;
	int max;
	int ratio;
}PRICERANGE;

template<class T1,class T2>
class static_hash_vector
{
public:
	static_hash_vector()
	{
		m_flag = false;		
		m_size = 0;
		m_data = NULL;	
		m_invalid.count = 0;
		m_invalid.data = NULL;
		m_invalid.size = 0;
		typename T2::value_type T2_type;
		string name = typeid(T2).name();
		if(name.find("vector") == string::npos)
		{
			m_type = T2_others;
			cout<<"T2 type error!"<<endl;
			return ;		
		}
		if(typeid(T2_type) == typeid(pair<int,int>))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(pair<int,int>);	
		}
		else if(typeid(T2_type) == typeid(pair<u64,int>))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(pair<u64,int>);
		}else if(typeid(T2_type) == typeid(string))
		{
			m_type = vector_string;
			m_type_size = STRMAXNUM;
		}
		else if (typeid(T2_type) == typeid(int))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(int);
		}
		else if (typeid(T2_type) == typeid(float))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(float);
		}
		else if(typeid(T2_type) == typeid(pair<string,int>))
		{
			m_type = vector_pair_string_int;
			m_type_size = sizeof(int)+STRMAXNUM;
		}
		else if(typeid(T2_type) == typeid(PRICERANGE))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(PRICERANGE);
		}
		else if(typeid(T2_type) == typeid(u64))
		{
			m_type = vector_fixed;
			m_type_size = sizeof(u64);
		}
		else
		{
			m_type = T2_others;
			cout<<"T2 type error!"<<endl;
		}
	}

	~static_hash_vector()
	{
		m_flag = false;
		m_size = 0;
		m_type_size = 0;
		//m_hash_map.clear();
		if(m_data != NULL)
		{
			delete []m_data;
			m_data = NULL;
		}
	}
	//将hashmap输出
	bool write(hash_map<T1,T2>& hash_map,const string& outfile);
	//读取文件
	bool read(const string& infile);
	
	HASHVECTOR operator[](const T1& key);

	inline unsigned int size()
	{
		return m_size;
	}
	private:
		bool m_flag;          //read 成功标志位
		int m_type_size;
		T2Type m_type;
		char *m_data;
		int m_size;
		HASHVECTOR m_invalid;
		//static_hash_map<T1,T2> m_fixed_hash;
		//hash_map<T1,pair<int,int> > m_hash_map;
		static_hash_map<T1,pair<int,int> > m_static_hash_map;
};

template<class T1,class T2>
bool static_hash_vector<T1, T2>::write(hash_map<T1,T2>& hashmap,const string& outfile)
{
	clock_t begin, end;
	double cost;

	int size;
	int count = 0;
	int offset = 0;                                                   //存储数据空间的大小 

	typename hash_map<T1,T2>::iterator iter; 
	string tail = ".dat";
	//异常防止
	if (m_type == T2_others)
	{
		cout<<"template is error!"<<endl;
		return false;
	}
	
	if (outfile == "")
	{
		cout<<"outfile is error!"<<endl;
		return false;
	}
	//m_hash_map.clear();
	hash_map<T1,pair<int,int> > tmp_hash;
	//打开输出文件
	tail = ".bin";
	FILE *pfile = fopen((outfile+tail).c_str(),"wb");
	if (pfile == NULL)
	{
		cout<<outfile+tail<<"open error!"<<endl;
		return false;
	}
	fseek(pfile,4,SEEK_SET);
 //////////////////////////////////////////////////////////////////////////////
	begin = clock();
	//将数据从哈希表中取出，建立哈希索引表，数据区存储
	//定长型数据
	if(m_type == vector_fixed)
	{		
		for (iter = hashmap.begin();iter!=hashmap.end();iter++)
		{	 
			size  = (iter->second).size();
			tmp_hash.insert(make_pair(iter->first,make_pair(size,offset)));  //建立索引哈希表
			fwrite(&(iter->second)[0],1,size*m_type_size,pfile);               //将数据写入文件
			fflush(pfile); 
			offset += size*m_type_size;                                      //更新偏移量
			count++;   
 		}
	}
	//将string转换为定长字符串
	else if (m_type == vector_string)
	{	
		int i;
		string *str;
		vector<string> *vtmp;
		for (iter = hashmap.begin();iter!=hashmap.end();iter++)
		{ 
			size  = (iter->second).size();
			tmp_hash.insert(make_pair(iter->first,make_pair(size,offset)));//建立索引哈希 ?
			for (i=0; i<size; i++)
			{
				str = (string*)&(iter->second)[i];
				str->resize(STRMAXNUM-1);         	
				fwrite(str->data(),1,m_type_size,pfile);    	 //将数据写入文件		
				fflush(pfile);
			}
			offset += size*m_type_size;                                      //更新偏移量			
			count++;   
		}
		str = NULL;
	} 
	else if(m_type == vector_pair_string_int)
	{
		int i;
		int *itmp;
		string *str;
		for (iter = hashmap.begin();iter!=hashmap.end();iter++)
		{
			size  = (iter->second).size();
			tmp_hash.insert( make_pair(iter->first,make_pair(size,offset)));//建立索引哈希表
			for (i=0; i<size; i++)
			{
				str = (string*)&(iter->second)[i];
				str->resize(STRMAXNUM-1);
				itmp = (int*)((char*)(&(iter->second)[i])+sizeof(string));
				fwrite(str->data(),1,STRMAXNUM,pfile);                     //将数据写入文件				
				fflush(pfile);
				fwrite(itmp,1,m_type_size-STRMAXNUM,pfile);               //将数据写入文件				
				fflush(pfile);
			}
			offset += size*m_type_size;                                      //更新偏移量			
			count++;   
		}
		str = NULL;
		itmp = NULL;
	}	
	fseek(pfile,0,SEEK_SET);											//在开始四字节写入数据空间大小			
	fwrite(&offset,4,1,pfile);
	fclose(pfile);

	end = clock();
	cost = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("data_bin time is %lf seconds\n", cost);
/////////////////////////////////////////////////////////////////////////////////
	m_size = offset;

	begin = clock();
	int bucket = (int)(ceil(log((double)count)/log(2.0)));                //计算所需桶数
	//tail = ".dat";
	//将索引哈希表存入文件
	if(false == m_static_hash_map.container_to_hash_file(tmp_hash,bucket,(outfile).c_str()))
	{
		cout<<"hash_map save failed!"<<endl;
		return false;
	}
	end = clock();
	cost = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("hash_dat time is %lf seconds\n", cost);

	return true;
}

template<class T1,class T2>
bool static_hash_vector<T1, T2>::read(const string& infile)
{
	if (m_type == T2_others)
	{
		cout<<"template is error!"<<endl;
		return false;
	}
	if (infile == "")
	{
		cout<<"infile is error!"<<endl;
		m_flag = false;	
		return false;
	}
	string tail = ".dat";

	//加载静态哈希索引表
	if(false == m_static_hash_map.load_serialized_hash_file((infile).c_str(),make_pair(-1,-1)))
	{
		cout<<"load static hashmap error!"<<endl;
		m_flag = false;
		return false;
	}
	//加载数据空间
	tail = ".bin";
	FILE *pfile = fopen((infile+tail).c_str(),"rb");
	if (pfile == NULL)
	{
		cout<<infile+tail<<"open error!"<<endl;
		m_flag = false;	
		return false;
	}
	int size;
	fread(&size,4,1,pfile);
	m_size = size;
	if (m_data != NULL)
	{
		delete [] m_data;
		m_data = NULL;
	}
	m_data = new char[size];
	if(size !=fread(m_data,1,size,pfile))
	{
		cout<<"file read error!"<<endl;
		m_flag = false;
		return false;
	}
	fclose(pfile);
	m_flag = true;
	return true;
}

template<class T1,class T2>
HASHVECTOR static_hash_vector<T1, T2>::operator[](const T1& key)
{
	//读取变长哈希
	pair<int,int> pVal;
	HASHVECTOR hash_struct;
	if (m_flag == false)
	{
		return m_invalid;
	}
	pVal = m_static_hash_map[key];
	if (pVal == make_pair(-1,-1))
	{
		return m_invalid;
	}
	hash_struct.count = pVal.first;
	hash_struct.data = m_data+ pVal.second;
	hash_struct.size = m_type_size;
	return hash_struct;
}
#endif
