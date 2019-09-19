#include <stdint.h>
inline uint8_t GetBits(uint8_t *ch, int nStart, int nCount)
{
	//nStart < 8 nCount < 8
	uint8_t ret;
	ret = ((ch[0] & 0xFF) << nStart) & 0xFF;
	if (nStart + nCount > 8)
	{
		ret = ((ret & 0xFF) >> nStart) & 0xFF;
		int nLeft = nStart + nCount - 8;
		ret = ((ret & 0xFF) << nLeft) & 0xFF;
		ret |= ((ch[1] & 0xFF) >> (8 - nLeft)) & 0xFF;
	}
	else
	{
		ret = ((ret & 0xFF) >> (8 - nCount)) & 0xFF;
	}
	return ret;
}

int ASC2ADTS(uint8_t *pAscData, int nAscLen, uint8_t *pAdtsData ,int nFrameSize)
{
	int nPosByte = 0;
	int nPosBit = 0;

	uint8_t objectType = (pAscData[nPosByte] & 0xF8) >> 3;
	nPosBit = 5;
	if (objectType == 31)
	{
		objectType = ((pAscData[nPosByte] & 0x7) << 3) |
			((pAscData[++nPosByte] & 0xE0) >> 5);
		nPosBit = 3;
	}
	uint8_t frequencyIndex = GetBits(pAscData + nPosByte, nPosBit, 4);
	int nFrequency = 0;
	nPosBit += 4;
	if (nPosBit > 7)
	{
		nPosByte++;
		nPosBit %= 8;
	}
	if (frequencyIndex == 15)
	{
		//´ó¶Ë×ªÐ¡¶Ë
		uint8_t *pFrequency = (uint8_t*)&nFrequency;
		pFrequency[2] = GetBits(pAscData + nPosByte, nPosBit, 8);
		nPosByte++;
		pFrequency[1] = GetBits(pAscData + nPosByte, nPosBit, 8);
		nPosByte++;
		pFrequency[0] = GetBits(pAscData + nPosByte, nPosBit, 8);
		nPosByte++;
	}
	uint8_t channelConfig = GetBits(pAscData + nPosByte, nPosBit, 4);
	nPosBit += 4;
	if (nPosBit > 7)
	{
		nPosByte++;
		nPosBit %= 8;
	}


	{
		pAdtsData[0] = 0xFF;
		pAdtsData[1] = 0xF1;//F8mpeg4 F0 mpeg2
		if (objectType > 0 && objectType < 4)
		{
			pAdtsData[2] = ((objectType - 1) & 0xFF) << 6;
		}
		else
		{
			pAdtsData[2] = 0xC0;
		}
		pAdtsData[2] |= (frequencyIndex << 2) & 0x3C;
		pAdtsData[2] |= (channelConfig >> 2) & 0x1;
		pAdtsData[3] = (channelConfig << 6) & 0xC0;
		nFrameSize += 7;
		pAdtsData[3] |= (nFrameSize >> 11) & 0x3;
		pAdtsData[4] = (nFrameSize >> 3) & 0xFF;
		pAdtsData[5] = (nFrameSize << 5) & 0xE0;
		pAdtsData[5] |= 0x1F;
		pAdtsData[6] = 0xFC;
	}


	return 1;
}
int main()
{
	uint8_t asc[7] = { 0x13 ,0x90 ,0x56 ,0xe5 ,0xa5 ,0x48 ,0x0 };
	uint8_t adts[7] = { 0 };
	ASC2ADTS(asc, 7, adts,101);
	return 0;
}