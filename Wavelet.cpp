#include "StdAfx.h"
#include ".\wavelet.h"

CWavelet::CWavelet(void)
{
	m_temp = new Vec3[32];//deb : not OOP!!!!
	memset(m_temp, 0, sizeof(Vec3) * 32);
	m_temp2 = new Vec3[32];
	memset(m_temp2, 0, sizeof(Vec3) * 32);
	m_temp1k = new Vec3[32*32];
	memset(m_temp1k, 0, sizeof(Vec3) * 32 * 32);

	m_fTemp = new float[32];//deb : not OOP!!!!
	memset(m_fTemp, 0, sizeof(float) * 32);
	m_fTemp2 = new float[32];
	memset(m_fTemp2, 0, sizeof(float) * 32);
	m_fTemp1k = new float[32*32];
	memset(m_fTemp1k, 0, sizeof(float) * 32 * 32);
}

CWavelet::~CWavelet(void)
{
	delete[] m_temp;
	m_temp = NULL;
	delete[] m_temp2;
	m_temp2 = NULL;
	delete[] m_temp1k;
	m_temp1k = NULL;

	delete[] m_fTemp;
	m_fTemp = NULL;
	delete[] m_fTemp2;
	m_fTemp2 = NULL;
	delete[] m_fTemp1k;
	m_fTemp1k = NULL;
}

#define _CWaveletOptimize
#ifdef	_CWaveletOptimize
//////////////////////////////////////////////////////Non-standard Haar2d in Eric paper, used for ng04 triple prod
void CWavelet::DecompositionStep(Vec3* array, int len)
{			
	for(int i = 0; i < len / 2; ++i)
	{
		m_temp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
		m_temp[i + len / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
	}
	memcpy(array, m_temp, len * sizeof(Vec3)); //len or just related????
}

void CWavelet::NonstandDecomposition(Vec3* mat, int rowNum, int colNum)//haar2D
{
	assert(rowNum == colNum);
	assert((rowNum & (rowNum - 1)) == 0);
	assert((colNum & (colNum - 1)) == 0);
	//Eric paper says /res, but that leads to wrong
	for(int related = rowNum; related > 1; related /= 2)
	{
		for(int i = 0; i < related; ++i)
		{	
			DecompositionStep(mat + i * colNum, related);//mat[i]
		}
		for(int i = 0; i < related; ++i)
		{
			for(int j = 0; j < related; ++j)//copy out mat[j][i]
				m_temp2[j] = *(mat + j * colNum + i);
			DecompositionStep(m_temp2, related);
			for(int j = 0; j < related; ++j)//copy in
				*(mat + j * colNum + i) = m_temp2[j];
		}
	}
}

void CWavelet::DecompositionStep(float* array, int len)
{			
	for(int i = 0; i < len / 2; ++i)
	{
		m_fTemp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
		m_fTemp[i + len / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
	}
	memcpy(array, m_fTemp, len * sizeof(float)); //len or just related????
}

void CWavelet::NonstandDecomposition(float* mat, int rowNum, int colNum)//haar2D
{
	assert(rowNum == colNum);
	assert((rowNum & (rowNum - 1)) == 0);
	assert((colNum & (colNum - 1)) == 0);
	//Eric paper says /res, but that leads to wrong
	for(int related = rowNum; related > 1; related /= 2)
	{
		for(int i = 0; i < related; ++i)
		{	
			DecompositionStep(mat + i * colNum, related);//mat[i]
		}
		for(int i = 0; i < related; ++i)
		{
			for(int j = 0; j < related; ++j)//copy out mat[j][i]
				m_fTemp2[j] = *(mat + j * colNum + i);
			DecompositionStep(m_fTemp2, related);
			for(int j = 0; j < related; ++j)//copy in
				*(mat + j * colNum + i) = m_fTemp2[j];
		}
	}
}
////////////////////////////////////////////////////////////////below are standard, can't be used in Non-linear
void CWavelet::Haar1D(Vec3* array, int len)
	{
		assert((len & (len - 1)) == 0);
		//memset(m_temp, 0, len * sizeof(Vec3));

		for(int related = len; related > 1; related /= 2)
		{
			for(int i = 0; i < related / 2; ++i)
			{
				m_temp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
				m_temp[i + related / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
			}
			memcpy(array, m_temp, len * sizeof(Vec3));
		}
	}
	void CWavelet::Haar2D(Vec3* mat, int rowNum, int colNum)//haar2D
	{
		assert((rowNum & (rowNum - 1)) == 0);
		assert((colNum & (colNum - 1)) == 0);


		for(int i = 0; i < rowNum; ++i)
		{	
			Haar1D(mat + i * colNum, colNum);//mat[i]
		}
		//memset(m_temp2, 0, rowNum * sizeof(Vec3));

		for(int i = 0; i < colNum; ++i)
		{
			for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
				m_temp2[j] = *(mat + j * colNum + i);
			Haar1D(m_temp2, rowNum);
			for(int j = 0; j < rowNum; ++j)//copy in
				*(mat + j * colNum + i) = m_temp2[j];
		}
	}


	//len: the number of elements to process. stride: the distance(in sizeof(element)) between elements. note that len is only relative elements number.
	void CWavelet::Haar1D(float* array, int len)
	{
		assert((len & (len - 1)) == 0);
		//memset(m_temp, 0, len * sizeof(Vec3));

		for(int related = len; related > 1; related /= 2)
		{
			for(int i = 0; i < related / 2; ++i)
			{
				m_fTemp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
				m_fTemp[i + related / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
			}
			memcpy(array, m_fTemp, len * sizeof(float)); //len or just related????
		}
	}

	void CWavelet::Haar2D(float* mat, int rowNum, int colNum)//haar2D
	{
		assert((rowNum & (rowNum - 1)) == 0);
		assert((colNum & (colNum - 1)) == 0);

		for(int i = 0; i < rowNum; ++i)
		{	
			Haar1D(mat + i * colNum, colNum);//mat[i]
		}
		//memset(m_fTemp2, 0, rowNum * sizeof(float));

		for(int i = 0; i < colNum; ++i)
		{
			for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
				m_fTemp2[j] = *(mat + j * colNum + i);
			Haar1D(m_fTemp2, rowNum);
			for(int j = 0; j < rowNum; ++j)//copy in
				*(mat + j * colNum + i) = m_fTemp2[j];
		}
	}

#else
	void CWavelet::Haar1D(Vec3* array, int len)
	{
		assert((len & (len - 1)) == 0);

		Vec3* temp = new Vec3[len];//need mem pool later
		for(int related = len; related > 1; related /= 2)
		{
			for(int i = 0; i < related / 2; ++i)
			{
				temp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
				temp[i + related / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
			}
			memcpy(array, temp, len * sizeof(Vec3)); //len or just related????
		}
		delete[] temp;
	}
	void CWavelet::Haar2D(Vec3* mat, int rowNum, int colNum)//haar2D
	{
		assert((rowNum & (rowNum - 1)) == 0);
		assert((colNum & (colNum - 1)) == 0);

		for(int i = 0; i < rowNum; ++i)
		{	
			Haar1D(mat + i * colNum, colNum);//mat[i]
		}

		Vec3* temp = new Vec3[rowNum];
		for(int i = 0; i < colNum; ++i)
		{
			for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
				temp[j] = *(mat + j * colNum + i);
			Haar1D(temp, rowNum);
			for(int j = 0; j < rowNum; ++j)//copy in
				*(mat + j * colNum + i) = temp[j];
		}
		delete[] temp;
	}

	//len: the number of elements to process. stride: the distance(in sizeof(element)) between elements. note that len is only relative elements number.
	void CWavelet::Haar1D(float* array, int len)
	{
		assert((len & (len - 1)) == 0);

		float* temp = new float[len];//need mem pool later
		for(int related = len; related > 1; related /= 2)
		{
			for(int i = 0; i < related / 2; ++i)
			{
				temp[i] = (array[i * 2] + array[i * 2 + 1]) * OneBySqrt2;
				temp[i + related / 2] = (array[i * 2] - array[i * 2 + 1]) * OneBySqrt2;
			}
			memcpy(array, temp, len * sizeof(float)); //len or just related????
		}
		delete[] temp;
	}

	void CWavelet::Haar2D(float* mat, int rowNum, int colNum)//haar2D
	{
		assert((rowNum & (rowNum - 1)) == 0);
		assert((colNum & (colNum - 1)) == 0);

		for(int i = 0; i < rowNum; ++i)
		{	
			Haar1D(mat + i * colNum, colNum);//mat[i]
		}

		float* temp = new float[rowNum];
		for(int i = 0; i < colNum; ++i)
		{
			for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
				temp[j] = *(mat + j * colNum + i);
			Haar1D(temp, rowNum);
			for(int j = 0; j < rowNum; ++j)//copy in
				*(mat + j * colNum + i) = temp[j];
		}
		delete[] temp;
	}

#endif

//////////////////////////////////////////////////////////////////////////not used

void CWavelet::ReverseHaar1D(Vec3* array, int len) //Haar reverse
{
	Vec3* temp = new Vec3[len];
	for(int related = 2; related <= len; related *= 2)
	{
		for (int i = 0; i < related / 2; ++i)
		{
			temp[2 * i] = (array[i] + array[i + related / 2]) * OneBySqrt2;
			temp[2 * i + 1] = (array[i] - array[i + related / 2]) * OneBySqrt2;
		}
		memcpy(array, temp, related * sizeof(Vec3));
	}
	delete[] temp;
}

void CWavelet::ReverseHaar2D(Vec3* mat, int rowNum, int colNum)//Haar2D reverse
{
	for(int i = 0; i < rowNum; ++i)
	{	
		ReverseHaar1D(mat + i * colNum, colNum);//mat[i]
	}

	Vec3* temp = new Vec3[rowNum];
	for(int i = 0; i < colNum; ++i)
	{
		for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
			temp[j] = *(mat + j * colNum + i);
		ReverseHaar1D(temp, rowNum);
		for(int j = 0; j < rowNum; ++j)//copy in
			*(mat + j * colNum + i) = temp[j];
	}
	delete[] temp;
}

void CWavelet::ReverseHaar1D(float* array, int len) //Haar reverse
{
	float* temp = new float[len];
	for(int related = 2; related <= len; related *= 2)
	{
		for (int i = 0; i < related / 2; ++i)
		{
			temp[2 * i] = (array[i] + array[i + related / 2]) * OneBySqrt2;
			temp[2 * i + 1] = (array[i] - array[i + related / 2]) * OneBySqrt2;
		}
		memcpy(array, temp, related * sizeof(float));
	}
	delete[] temp;
}

void CWavelet::ReverseHaar2D(float* mat, int rowNum, int colNum)//Haar2D reverse
{
	for(int i = 0; i < rowNum; ++i)
	{	
		ReverseHaar1D(mat + i * colNum, colNum);//mat[i]
	}

	float* temp = new float[rowNum];
	for(int i = 0; i < colNum; ++i)
	{
		for(int j = 0; j < rowNum; ++j)//copy out mat[j][i]
			temp[j] = *(mat + j * colNum + i);
		ReverseHaar1D(temp, rowNum);
		for(int j = 0; j < rowNum; ++j)//copy in
			*(mat + j * colNum + i) = temp[j];
	}
	delete[] temp;
}



////for test Haar2D
//float a[16], b[16], sum1 =0, sum2 = 0;
//for(int i = 0; i < 16; ++i)
//a[i] = rand();
//for(int i = 0; i < 16; ++i)
//b[i] = rand();
//
//for(int i = 0; i < 16; ++i)
//sum1 += a[i]*b[i];
//CWavelet w;
//w.Haar2D(a, 4,4);
//w.Haar2D(b, 4,4);
//for(int i = 0; i < 16; ++i)
//sum2 += a[i]*b[i];

////for test Haar2D
//CImage img;
//img.Load("test.jpg");//col and row can be different
//
////new mat
//int imgW =  img.GetWidth();
//int imgH =  img.GetHeight();
//Vec3** mat = new Vec3*[imgH];
//for(int i = 0; i < imgH; ++i)
//mat[i] = new Vec3[imgW];
//
////read in img
//for(int i = 0; i < imgW; i++)
//for(int j = 0; j < imgH; ++j)
//{
//	COLORREF col = img.GetPixel(i, j);
//	mat[i][j] = Vec3((float)GetRValue(col), (float)GetGValue(col), (float)GetBValue(col));
//}
//
////img compress
//m_wavelet.Haar2D(mat, imgH, imgW);
//
////write back img
//for(int i = 0; i < imgW; i++)
//for(int j = 0; j < imgH; ++j)
//img.SetPixelRGB(i, j, (byte)mat[i][j].x, (byte)mat[i][j].y, (byte)mat[i][j].z);
//
//img.Save("testHaar2D.jpg");
//img.Destroy();

////for test ReverseHaar2D
//float mat[16] = {0.9501,0.8913, 0.8214,0.9218,
//0.2311, 0.7621,    0.4447    ,0.7382,
//0.6068    ,0.4565    ,0.6154    ,0.1763,
//0.4860    ,0.0185    ,0.7919    ,0.4057};
//m_wavelet.Haar2D(mat, 4,4);
//m_wavelet.ReverseHaar2D(mat, 4,4);
