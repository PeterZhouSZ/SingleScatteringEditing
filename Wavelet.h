#pragma once
#include "assert.h"
#include "math.h"
#include <algorithm>
#include <vector>

#include "3Dmath.h"

#define OneBySqrt2 0.70710678118654752440084436210485f
#define AT(f,r,c) ((f)[(r)*n+(c)])

class CWavelet
{
public:
	CWavelet(void);
	~CWavelet(void);
	////////////////////////////////////////////////////////////////////////// new Non-standard
	void DecompositionStep(float* array, int len);//haar1D
	void NonstandDecomposition(float* mat, int rowNum, int colNum);//haar2D
	void DecompositionStep(Vec3* array, int len);//haar1D
	void NonstandDecomposition(Vec3* mat, int rowNum, int colNum);//haar2D

	//////////////////////////////////////////////////////////////////////////standard
	void Haar1D(float* array, int len);

	void ReverseHaar1D(float* array, int len); //Haar reverse

	void Haar2D(float* mat, int rowNum, int colNum);//haar2D reverse

	void ReverseHaar2D(float* mat, int rowNum, int colNum);//Haar2D reverse

	//////////////////////////////////////////////////////////////////////////

	void Haar1D(Vec3* array, int len);

	void ReverseHaar1D(Vec3* array, int len); //Haar reverse

	void Haar2D(Vec3* mat, int rowNum, int colNum);//haar2D reverse

	void ReverseHaar2D(Vec3* mat, int rowNum, int colNum);//Haar2D reverse

	inline void haar2d(Vec3 *fi, Vec3* fo, const int n)
	{
		int lv,sz,i,j;
		Vec3 a,b,c,d;
		for(lv=1,sz=n;lv<n;lv*=2)
		{
			sz/=2;
			for(i=0;i<sz;i++)
				for(j=0;j<sz;j++)
				{
					a=AT(fi,i*2,j*2);
					b=AT(fi,i*2+1,j*2);
					c=AT(fi,i*2,j*2+1);
					d=AT(fi,i*2+1,j*2+1);
					AT(fi,i,j)=(a+b+c+d)/2.0f;
					AT(fo,i+sz,j)=(a-b+c-d)/2.0f;
					AT(fo,i,j+sz)=(a+b-c-d)/2.0f;
					AT(fo,i+sz,j+sz)=(a-b-c+d)/2.0f;
				}
		}
		*fo=*fi;//1st item copy, not pointer copy!!!
	}

protected:
	Vec3* m_temp; //we can use this instead of "temp" above, in performance stage. but i misused it and get troubled.... ^%$#&^%&(*^&(&*^%%%R#$@!$#@!$E#@RAaaaaaaaaaaa~~~~~~~
	Vec3* m_temp2;
	Vec3* m_temp1k;
	float* m_fTemp;
	float* m_fTemp2;
	float* m_fTemp1k;
};

inline bool _idxLess(std::pair<float, int> first, std::pair<float, int> second)
{
	return first.second < second.second;
};

inline bool _coefFabsGreater(const std::pair<float, short int> &first, const std::pair<float, short int>& second)
{
	return fabs(first.first) > fabs(second.first);
};

//keep only nth_element bigger former part of coefs, copy them into bigCoefs, track each's indices into coefIndices.
inline void KeepLoss(Vec3* array, int num, int keepNum, Vec3* bigElements, short int* elementIndices)
{
	std::vector<std::pair<float, short int> > CoefAndIdx(num);	
	
	for(short int i = 0; i < (short int)num; ++i)	
		CoefAndIdx[i] = std::pair<float, short int>(0.299f * array[i].x + 0.587f * array[i].y + 0.114f * array[i].z , i);
	//deb CoefAndIdx[i] = std::pair<float, unsigned int>(Norm(array[i]), i);

	std::nth_element(CoefAndIdx.begin(), CoefAndIdx.begin() + keepNum, CoefAndIdx.end(), _coefFabsGreater);//cut off: first 128 is biggest

	//when wavelet num can vary, add this line
	std::sort(CoefAndIdx.begin(), CoefAndIdx.begin() + keepNum, _coefFabsGreater);//for real-time flexible choice of num of LIGHT coefs, sort these coefs decreasely

	//bigCoefs and corresponding indices in the same position in coefIndices
	for (int i = 0; i < keepNum; ++i)
	{
		bigElements[i] = array[CoefAndIdx[i].second];
		elementIndices[i] = CoefAndIdx[i].second;
	}
}

//keep only nth_element bigger former part of coefs, copy them into bigCoefs, track each's indices into coefIndices.
inline void KeepLoss(float* array, int num, int keepNum, float* bigElements, short int* elementIndices)
{
	std::vector<std::pair<float, short int> > CoefAndIdx(num);	

	for(short int i = 0; i < (short int)num; ++i)	
		CoefAndIdx[i] = std::pair<float, short int>(array[i], i);

	std::nth_element(CoefAndIdx.begin(), CoefAndIdx.begin() + keepNum, CoefAndIdx.end(), _coefFabsGreater);//cut off: first 128 is biggest

	//when wavelet num can vary, add this line
	std::sort(CoefAndIdx.begin(), CoefAndIdx.begin() + keepNum, _coefFabsGreater);//for real-time flexible choice of num of LIGHT coefs, sort these coefs decreasely

	//bigCoefs and corresponding indices in the same position in coefIndices
	for (int i = 0; i < keepNum; ++i)
	{
		bigElements[i] = CoefAndIdx[i].first;
		elementIndices[i] = CoefAndIdx[i].second;
	}
}

inline int log2(int x)
{
	int i = 0;
	while((x = x >> 1) > 0)
		++i;
	return i;
}

//keep only nth_element bigger former part of area-weighted cube coefs, copy orig coefs into bigCoefs, track each's indices into coefIndices.
inline void CubeKeepLoss(const float* array, const int cubeRes, const int keepNum, float* bigElements, short int* elementIndices, bool bAreaWeighting, bool bSortByCoef, bool bSortByIdx)
{
#define max(a,b) (((a) > (b)) ? (a) : (b))
	int sideNum = cubeRes * cubeRes;
	std::vector<std::pair<float, short int> > WCoefAndIdx(sideNum * 6);	
	for(short int i = 0; i < (short int)sideNum*6; ++i)	
	{
		//effect too bad!! WCoefAndIdx[i] = std::pair<float, short int>(array[i], i);
		if(bAreaWeighting)
			WCoefAndIdx[i] = std::pair<float, short int>(array[i] / 
				(1 << log2(max((i % sideNum) % cubeRes, (i % sideNum) / cubeRes))), i);
		else
			WCoefAndIdx[i] = std::pair<float, short int>(array[i], i);
	}
	std::nth_element(WCoefAndIdx.begin(), WCoefAndIdx.begin() + keepNum, WCoefAndIdx.end(), _coefFabsGreater);//cut off: first 128 is biggest

	//when wavelet num can vary, add this line
	if(bSortByCoef)
		std::sort(WCoefAndIdx.begin(), WCoefAndIdx.begin() + keepNum, _coefFabsGreater);//for real-time flexible choice of num of LIGHT coefs, sort these coefs decreasely
	else if(bSortByIdx)
		std::sort(WCoefAndIdx.begin(), WCoefAndIdx.begin() + keepNum, _idxLess);//for real-time 3p indexing, sort into s/l/i/j/m increasing order. Refer to Ng04

	//bigCoefs and corresponding indices in the same position in coefIndices
	for (int i = 0; i < keepNum; ++i)
	{
		bigElements[i] = array[WCoefAndIdx[i].second];
		elementIndices[i] = WCoefAndIdx[i].second;
	}
}

//keep only nth_element bigger former part of area-weighted cube coefs, copy orig coefs into bigCoefs, track each's indices into coefIndices.
inline void WeightedCubeKeepLossNoSort(Vec3* array, int cubeRes, int keepNum, Vec3* bigElements, short int* elementIndices)
{
#define max(a,b) (((a) > (b)) ? (a) : (b))
	int sideNum = cubeRes * cubeRes;
	std::vector<std::pair<float, short int> > WCoefAndIdx(sideNum * 6);	
	for(short int i = 0; i < (short int)sideNum*6; ++i)	
	{
		//int test = (1 << log2(max((i % sideNum) % cubeRes, (i % sideNum) / cubeRes)));
		WCoefAndIdx[i] = std::pair<float, short int>((0.299f * array[i].x + 0.587f * array[i].y + 0.114f * array[i].z ) / 
			(1 << log2(max((i % sideNum) % cubeRes, (i % sideNum) / cubeRes))), i);
	}
	std::nth_element(WCoefAndIdx.begin(), WCoefAndIdx.begin() + keepNum, WCoefAndIdx.end(), _coefFabsGreater);//cut off: first 128 is biggest

	//when wavelet num can vary, add this line
	//std::sort(AreaWeightedCoefAndIdx.begin(), AreaWeightedCoefAndIdx.begin() + keepNum, _coefFabsGreater);//for real-time flexible choice of num of LIGHT coefs, sort these coefs decreasely

	//bigCoefs and corresponding indices in the same position in coefIndices
	for (int i = 0; i < keepNum; ++i)
	{
		bigElements[i] = array[WCoefAndIdx[i].second];
		elementIndices[i] = WCoefAndIdx[i].second;
	}
}


inline void haar2d(float *fi,float *fo,const int n)
{
	int lv,sz,i,j;
	float a,b,c,d;

	for(lv=1,sz=n;lv<n;lv*=2)
	{
		sz/=2;
		for(i=0;i<sz;i++)
			for(j=0;j<sz;j++)
			{
				a=AT(fi,i*2,j*2);
				b=AT(fi,i*2+1,j*2);
				c=AT(fi,i*2,j*2+1);
				d=AT(fi,i*2+1,j*2+1);
				AT(fi,i,j)=(a+b+c+d)/2.0f;
				AT(fo,i+sz,j)=(a-b+c-d)/2.0f;
				AT(fo,i,j+sz)=(a+b-c-d)/2.0f;
				AT(fo,i+sz,j+sz)=(a-b-c+d)/2.0f;
			}
	}
	*fo=*fi;
}
#undef AT
