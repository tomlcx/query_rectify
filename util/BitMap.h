#ifndef _BITMAP_H
#define _BITMAP_H
#include <vector>
using namespace std;

class CBitMap
{

public:
	CBitMap(unsigned int n)
	{
		unsigned int resCnt = n/8;
		if(n%8>0)
			++resCnt;
		m_v.resize(resCnt,0);
		m_max = n;
	}
	CBitMap()
	{
		m_max = 0;
	}

	inline bool Init(unsigned int n)
	{
		if(n == 0) 
		{
			m_max = 0;
			return false;
		}
		else 
			m_max = n ;
		unsigned int resCnt = n/8;
		if(n%8>0)
			++resCnt;
		m_v.resize(resCnt,0);
		ClearAll();
		return true;
	}

	inline int GetBitCount(){return m_v.size()<<3;}
	inline void SetBit(unsigned int n){m_v[n>>3] |= (1<<(n&7));}
	inline bool TestBit(unsigned int n){return (m_v[n>>3] & (1<<(n&7)))!=0;}
	inline void ClearBit(unsigned int n){m_v[n>>3] &= (~(1<<(n&7)));}
	inline void ClearAll(){memset(&m_v[0],0,m_v.size());}

	inline void SetBitSafe(unsigned int n)
	{
		if(n > m_max) return ;
		m_v[n>>3] |= (1<<(n&7));
	}
	inline bool TestBitSafe(unsigned int n)
	{
		if(n > m_max) return false;
		return (m_v[n>>3] & (1<<(n&7)))!=0;
	}
	inline void ClearBitSafe(unsigned int n)
	{
		if(n > m_max) return;
		m_v[n>>3] &= (~(1<<(n&7)));
	}

private:
	unsigned int m_max;
	vector<unsigned char> m_v;//m_v[0]:0~7 m_v[1]:8~15 ...
};
#endif
