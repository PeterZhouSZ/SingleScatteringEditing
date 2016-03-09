#include <iostream>
#include "Wavelet.h"
#include "atlimage.h"
using namespace std;
//6sides, each side res*res
#define MyRes 32
#define n1k 1024	
#define n6k 6144
#define keepN 256 //this time, means ALL 6 sides totals up to 256

//get Coef from Xmat, with s=(level,iIdx,jIdx), M=01/10/11
const float& GetCoef(float* Xmat, int level, int iIdx, int jIdx, int MIdx, int res)
{
	switch(MIdx)//res * row + col
	{
	case 1://M01. col: 2^l +i, row: j
		return *(Xmat + res * jIdx + (1<<level) + iIdx);
		break;
	case 2://M10. col: i, row: 2^l+j
		return *(Xmat + res * ((1<<level) + jIdx) + iIdx);
	case 3://M11. col: 2^l+i, row: 2^l+j
		return *(Xmat + res * ((1<<level) + jIdx) + (1<<level) + iIdx);
	default:
		assert(0);
		break;
	}
	return Xmat[0];
}

//get Coef from Xmat, with s=(level,iIdx,jIdx), M=01/10/11
const float& GetCoefSide(int side, float* Xmat, int level, int iIdx, int jIdx, int MIdx, int res)
{
	switch(MIdx)//res * row + col
	{
	case 1://M01. col: 2^l +i, row: j
		return *(Xmat + res * jIdx + (1<<level) + iIdx + side*res*res);
		break;
	case 2://M10. col: i, row: 2^l+j
		return *(Xmat + res * ((1<<level) + jIdx) + iIdx + side*res*res);
	case 3://M11. col: 2^l+i, row: 2^l+j
		return *(Xmat + res * ((1<<level) + jIdx) + (1<<level) + iIdx + side*res*res);
	default:
		assert(0);
		break;
	}
	return Xmat[0];//never comes here
}

const float GetCoefNL(float* vBig, short int* vBigIdx, int level, int iIdx, int jIdx, int MIdx, int res)
{
	int idx;
	switch(MIdx)//res * row + col
	{
	case 1://M01. col: 2^l +i, row: j
		idx = res * jIdx + (1<<level) + iIdx;
		break;
	case 2://M10. col: i, row: 2^l+j
		idx = res * ((1<<level) + jIdx) + iIdx;
		break;
	case 3://M11. col: 2^l+i, row: 2^l+j
		idx = res * ((1<<level) + jIdx) + (1<<level) + iIdx;
		break;
	default:
		assert(0);
		break;
	}
	for(int i = 0; i < keepN; ++i)
	{
		if(vBigIdx[i] == idx)
			return vBig[i];
	}
	return 0;//this is important!
}

const float GetCoefNLside(int side, float* vBig, short int* vBigIdx, int level, int iIdx, int jIdx, int MIdx, int res)
{
	int idx;
	switch(MIdx)//res * row + col
	{
	case 1://M01. col: 2^l +i, row: j
		idx = res * jIdx + (1<<level) + iIdx;
		break;
	case 2://M10. col: i, row: 2^l+j
		idx = res * ((1<<level) + jIdx) + iIdx;
		break;
	case 3://M11. col: 2^l+i, row: 2^l+j
		idx = res * ((1<<level) + jIdx) + (1<<level) + iIdx;
		break;
	default:
		assert(0);
		break;
	}
	idx += side * res * res;
	for(int i = 0; i < keepN; ++i)
	{
		if(vBigIdx[i] == idx)
			return vBig[i];
	}
	return 0;//this is important!
}

int SignOfQuadrant(int MIdx, int qx, int qy)
{
	if(MIdx == 1)
	{
		if(qx == 0)	return 1;
		else return -1;
	}
	if(MIdx == 2)
	{
		if(qy == 0) return 1;//might wrong
		else return -1;
	}
	if(MIdx == 3)
	{
		if(qx == 0)
		{
			if(qy == 0) return 1;
			else return -1;
		}
		else
		{
			if(qy == 0) return -1;
			else return 1;
		}
	}
	assert(0);
	return 0;
}

const float GetCoefPSum(float* Xmat, int level, int iIdx, int jIdx, int res)
{
	if((level == 0) && (iIdx == 0) && (jIdx == 0))
		return Xmat[0];
	int ol = level - 1;
	int oi = iIdx / 2;
	int oj = jIdx / 2;
	int qx = iIdx - 2 * oi; 
	int qy = jIdx - 2 * oj;
	return GetCoefPSum(Xmat, ol, oi, oj, res) + (1 << ol) *
		(GetCoef(Xmat, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
		 GetCoef(Xmat, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
		 GetCoef(Xmat, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
}
const float GetCoefPSumSide(int side, float* Xmat, int level, int iIdx, int jIdx, int res)
{
	if((level == 0) && (iIdx == 0) && (jIdx == 0))
		return Xmat[side * res * res];
	int ol = level - 1;
	int oi = iIdx / 2;
	int oj = jIdx / 2;
	int qx = iIdx - 2 * oi; 
	int qy = jIdx - 2 * oj;
	return GetCoefPSumSide(side, Xmat, ol, oi, oj, res) + (1 << ol) *
		(GetCoefSide(side, Xmat, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
		 GetCoefSide(side, Xmat, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
		 GetCoefSide(side, Xmat, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
}

const float GetCoefPSumNL(float* vBig, short int* vBigIdx, int level, int iIdx, int jIdx, int res)
{
	if((level == 0) && (iIdx == 0) && (jIdx == 0))
		return vBig[0];
	int ol = level - 1;
	int oi = iIdx / 2;
	int oj = jIdx / 2;
	int qx = iIdx - 2 * oi; 
	int qy = jIdx - 2 * oj;
	return GetCoefPSumNL(vBig, vBigIdx, ol, oi, oj, res) + (1 << ol) *
		(GetCoefNL(vBig, vBigIdx, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
		 GetCoefNL(vBig, vBigIdx, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
		 GetCoefNL(vBig, vBigIdx, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
}

const float GetCoefPSumNLside(int side, float* vBig, short int* vBigIdx, int level, int iIdx, int jIdx, int res)
{
	if((level == 0) && (iIdx == 0) && (jIdx == 0))
	{
		int idx = side * res * res;
		for(int i = 0; i < keepN; ++i)
		{
			if(vBigIdx[i] == idx)
				return vBig[i];
		}
		return 0;//this is important!
	}
	int ol = level - 1;
	int oi = iIdx / 2;
	int oj = jIdx / 2;
	int qx = iIdx - 2 * oi; 
	int qy = jIdx - 2 * oj;
	return GetCoefPSumNLside(side, vBig, vBigIdx, ol, oi, oj, res) + (1 << ol) *
		(GetCoefNLside(side, vBig, vBigIdx, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
		GetCoefNLside(side, vBig, vBigIdx, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
		GetCoefNLside(side, vBig, vBigIdx, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
}
void main()
{
	CWavelet m_wavelet;
	const int res = MyRes;

	float Lmat[n6k];
	for(int i = 0; i < n6k; ++i)
		Lmat[i] = float(rand()%100);//(float)i;
	float Pmat[n6k];
	for(int i = 0; i < n6k; ++i)
		Pmat[i] = float(rand()%201);//Lmat[i] * 10;
	float Tmat[n6k];
	for(int i = 0; i < n6k; ++i)
		Tmat[i] = float(rand()%301);//Lmat[i] * 100;

	float sum1=0, sum2=0, sum3=0, sum4=0;
	for(int i = 0; i < n6k; ++i)
	{
		sum1 += Lmat[i] * Pmat[i];
		sum2 += Lmat[i] * Tmat[i];
		sum3 += Pmat[i] * Tmat[i];
		sum4 += Lmat[i] * Pmat[i] * Tmat[i];
	}
	for(int side = 0; side < 6; ++side)
	{
		m_wavelet.NonstandDecomposition(&Lmat[side * n1k], res, res);
		m_wavelet.NonstandDecomposition(&Pmat[side * n1k], res, res);
		m_wavelet.NonstandDecomposition(&Tmat[side * n1k], res, res);
	}

	vector<float> vBigP(keepN, 0.f);
	vector<float> vBigT(keepN, 0.f);
	vector<short int> vBigPIdx(keepN, 0);
	vector<short int> vBigTIdx(keepN, 0);
	//KeepLoss(Pmat, n6k, keepN, &vBigP[0], &vBigPIdx[0]);
	WeightedCubeKeepLoss(Pmat, MyRes, keepN, &vBigP[0], &vBigPIdx[0]);
	//KeepLoss(Tmat, n6k, keepN, &vBigT[0], &vBigTIdx[0]);
	WeightedCubeKeepLoss(Tmat, MyRes, keepN, &vBigT[0], &vBigTIdx[0]);

	float sum1c=0, sum2c=0, sum3c=0, sum4c=0;
	for(int i = 0; i < n6k; ++i)
	{
		sum1c += Lmat[i] * Pmat[i];//using Nonstand, 0.00001 precision holds for double product:)
		sum2c += Lmat[i] * Tmat[i];
		sum3c += Pmat[i] * Tmat[i];
		sum4c += Lmat[i] * Pmat[i] * Tmat[i];
	}
	float integral = 0;
	float Cuvw = 0;
	int tmp = 1, levelNum = 0;
	while(tmp != n1k)
	{
		tmp *= 4;
		++levelNum;
	}
	float psumL = 0, psumP = 0, psumT = 0;
	float PscaleOneSide = 0, TscaleOneSide = 0;
	for(int side = 0; side < 6; ++side)
	{
		//deb integral += Lmat[side * n1k] * Pmat[side * n1k] * Tmat[side * n1k];
		for(int i = 0; i < keepN; ++i)
		{
			if(vBigPIdx[i] == side * n1k)
			{
				PscaleOneSide = vBigP[i];	
				break;
			}
		}
		for(int i = 0; i < keepN; ++i)
		{
			if(vBigTIdx[i] == side * n1k)
			{
				TscaleOneSide = vBigT[i];	
				break;
			}
		}
		integral += Lmat[side * n1k] * PscaleOneSide * TscaleOneSide;
		for(int level = 0; level < levelNum; ++level)
			for(int iIdx = 0; iIdx < (1<<level); ++iIdx)
				for(int jIdx = 0; jIdx < (1<<level); ++jIdx)
				{
				Cuvw = float(1 << level);
////deb
//				float L1 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 1, res);
//				float L2 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 2, res);
//				float L3 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 3, res);
//				float P1 = GetCoefSide(side, &Pmat[0], level, iIdx, jIdx, 1, res);
//				float P2 = GetCoefSide(side, &Pmat[0], level, iIdx, jIdx, 2, res);
//				float P3 = GetCoefSide(side, &Pmat[0], level, iIdx, jIdx, 3, res);
//				float T1 = GetCoefSide(side, &Tmat[0], level, iIdx, jIdx, 1, res);
//				float T2 = GetCoefSide(side, &Tmat[0], level, iIdx, jIdx, 2, res);
//				float T3 = GetCoefSide(side, &Tmat[0], level, iIdx, jIdx, 3, res);

				float L1 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 1, res);
				float L2 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 2, res);
				float L3 = GetCoefSide(side, &Lmat[0], level, iIdx, jIdx, 3, res);
				float P1 = GetCoefNLside(side, &vBigP[0], &vBigPIdx[0], level, iIdx, jIdx, 1, res);
				float P2 = GetCoefNLside(side, &vBigP[0], &vBigPIdx[0], level, iIdx, jIdx, 2, res);
				float P3 = GetCoefNLside(side, &vBigP[0], &vBigPIdx[0], level, iIdx, jIdx, 3, res);
				float T1 = GetCoefNLside(side, &vBigT[0], &vBigTIdx[0], level, iIdx, jIdx, 1, res);
				float T2 = GetCoefNLside(side, &vBigT[0], &vBigTIdx[0], level, iIdx, jIdx, 2, res);
				float T3 = GetCoefNLside(side, &vBigT[0], &vBigTIdx[0], level, iIdx, jIdx, 3, res);
				//case 2
				integral += Cuvw * (
					P1 * L2 * T3 +
					P1 * L3 * T2 +
					P2 * L3 * T1 +
					P2 * L1 * T3 +
					P3 * L1 * T2 +
					P3 * L2 * T1);
				//case 3
////deb
//				psumL = GetCoefPSumSide(side, &Lmat[0], level, iIdx, jIdx, res);
//				psumP = GetCoefPSumSide(side, &Pmat[0], level, iIdx, jIdx, res);
//				psumT = GetCoefPSumSide(side, &Tmat[0], level, iIdx, jIdx, res);
				psumL = GetCoefPSumSide(side, &Lmat[0], level, iIdx, jIdx, res);
				psumP = GetCoefPSumNLside(side, &vBigP[0], &vBigPIdx[0], level, iIdx, jIdx, res);
				psumT = GetCoefPSumNLside(side, &vBigT[0], &vBigTIdx[0], level, iIdx, jIdx, res);
				integral +=
					(P1 * L1 * psumT +
					 L1 * T1 * psumP +
					 T1 * P1 * psumL);
				integral +=
					(P2 * L2 * psumT +
					L2 * T2 * psumP +
					T2 * P2 * psumL);
				integral +=
					(P3 * L3 * psumT +
					L3 * T3 * psumP +
					T3 * P3 * psumL);
			}
	}
	integral /= res;  //should do this to normalize original sum4 energy
	cout<<"hello"<<endl;	
}