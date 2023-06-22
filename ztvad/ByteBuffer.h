/*
	ByteBuffer class
	Written by Gao Peng
	Version 1.3

	buffering data chunk with any size

*/


#if !defined(_BYTE_BUFFER_H_)
#define _BYTE_BUFFER_H_

#include <string.h>


class ByteBuffer
{
public:
	ByteBuffer();
	~ByteBuffer();

	bool Init(int nBufInitSize, bool bAutoInc=false, int nMaxBufSize=8*1048576);
	void Reset();
	void Clear();

	void SetAutoInc(bool bAutoInc) { m_bAutoInc = bAutoInc; }
	bool IsAutoInc() { return m_bAutoInc; }

	bool Feed(const char *pBytes, int &nLen);
	char *GetBytes() { return m_pBuffer; }
	int GetByteNum() { return m_nBufPos; }
	void SetByteNum(int nByteNum)
		{ if (nByteNum>=0 && nByteNum<=m_nBufSize) m_nBufPos = nByteNum; }
	bool IsFull() { return (m_pBuffer && m_nBufPos == m_nBufSize); }
	bool Shift(int nLen);

	int GetBufSize() { return m_nBufSize; }
	int GetMaxBufSize() { return m_nMaxBufSize; }

private:
	char	*m_pBuffer;
	int		m_nBufSize;
	bool	m_bAutoInc;
	int		m_nBufPos;
	int		m_nMaxBufSize;

	float	m_fIncRatio;
};


inline ByteBuffer::ByteBuffer()
{
	m_pBuffer = NULL;
	m_nBufSize = 0;

	m_fIncRatio = 0.25;
}

inline ByteBuffer::~ByteBuffer()
{
	Reset();
}

inline bool ByteBuffer::Init(int nBufInitSize, bool bAutoInc, int nMaxBufSize)
{
	if (nBufInitSize <= 0 || nMaxBufSize <= 0 || nMaxBufSize < nBufInitSize)
		return false;
	if (m_nBufSize == nBufInitSize) {
		m_bAutoInc = bAutoInc;
		m_nMaxBufSize = nMaxBufSize;
		Clear();
		return true;
	}
	Reset();
	m_pBuffer = new char[nBufInitSize];
	if (!m_pBuffer)
		return false;
	m_nBufSize = nBufInitSize;
	m_bAutoInc = bAutoInc;
	m_nMaxBufSize = nMaxBufSize;
	Clear();
	return true;
}

inline void ByteBuffer::Reset()
{
	Clear();
	if (m_pBuffer) {
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

inline void ByteBuffer::Clear()
{
	m_nBufPos = 0;
}

inline bool ByteBuffer::Feed(const char *pData, int &nLen)
{
	if (!pData || nLen < 0 || !m_pBuffer)
		return false;
	int	nUsable = nLen;
    if (m_nBufPos+nLen > m_nBufSize) {
		if (m_bAutoInc) {
			int	nNewSize = m_nBufPos+nLen;
			if (nNewSize > m_nMaxBufSize)
				return false;

			//if (m_nBufSize < 500*1024*1024) {
			//	if (nNewSize < m_nBufSize * 5/4) {
			//		nNewSize = m_nBufSize * 5/4;
			//		if (nNewSize > m_nMaxBufSize)
			//			nNewSize = m_nMaxBufSize;
			//	}
			//}
			if (nNewSize < m_nBufSize * (1+m_fIncRatio))
				nNewSize = (int)(m_nBufSize * (1+m_fIncRatio));
			if (nNewSize > m_nMaxBufSize)
				nNewSize = m_nMaxBufSize;

			char	*pNewBuf = new char[nNewSize];
			if (pNewBuf) {
				memcpy(pNewBuf, m_pBuffer, m_nBufPos);
				delete[] m_pBuffer;
				m_pBuffer = pNewBuf;
				m_nBufSize = nNewSize;
			}
			else
				return false;
		}
        else {
			nUsable = m_nBufSize - m_nBufPos;
        }
    }
	memcpy(m_pBuffer+m_nBufPos, pData, nUsable);
	nLen -= nUsable;
	m_nBufPos += nUsable;
	return true;
}

inline bool ByteBuffer::Shift(int nLen)
{
	if (!m_pBuffer || nLen < 0)
		return false;
	if (nLen == 0)
		return true;
	else if (nLen >= m_nBufPos) {
		m_nBufPos = 0;
		return true;
	}
	memmove(m_pBuffer, m_pBuffer+nLen, m_nBufPos-nLen);
	m_nBufPos -= nLen;
	return true;
}


#endif
