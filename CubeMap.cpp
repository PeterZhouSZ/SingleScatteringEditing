#include "StdAfx.h"
#include <FSTREAM>
#include ".\cubemap.h"

#include <gl\gl.h>
#include <gl\glu.h>
#include "gl\glext.h"

/* OpenGL Cube atlas
	y
	A
	|
	/----> x
   /
  z
				____________
				|     Up(2) |
				| t  		|
				| |         |
				| |____s    |
	____________|___________|________________________
   |	   s____|	   s____| ____s	    |	 s____  |
   |		   ||		   |||		    |		 |  |		
   |		   ||          |||          |		 |  |
   |Left(1)    t|Front(5)  t|t Right(0) |Back(4) t  |
   |____________|___________|___________|___________|
				|    Down(3)|
				|t          |
				||          |
				||___s      |
				|___________|

fold inside this cross to get the cube. light env map is drawn in this way, and vertex's transport vector is also arranged in this way.
*/



CCubeMap::CCubeMap(void)
{
	m_pCube = NULL;
	m_pDownCube = NULL;
	m_downRes = 0;
	m_nFaceWidth = 0;
	m_nFaceHeight = 0;
}

CCubeMap::~CCubeMap(void)
{	
	if(m_pCube)
		delete [] m_pCube;
	if(m_pDownCube)
		delete[] m_pDownCube;
}



//Light offset to light direction: given a (trans vector or light tex image) memory offset, return corresponding light dir and solid angle
 Vec3 CCubeMap::Offset2Dir(int side, int offset, int res, float& solidAngle)
{
	//since cube light is not evenly sampled, a light sample should be weighted by its solid angle: L*dw. 
	//dw = dA*cos/(r*r), r = Const/cos (since r*cos=Const), so dw = Const / (r*r*r). so L is weighted by 1 / r^3.
	//r^3 = (a^2 + (i-a)^2 + (j-a)^2)^(3/2), where a is cubeSize/2, i,j is sample's coordinate

	//[0,31] -> (-1,1)
#define A(i, res) ((i + 0.5f) * 2.f / res - 1.f)

	int i = offset % res;
	int j = offset / res;
	float x = 0.f, y = 0.f, z = 0.f, di = 0.f, dj = 0.f;
	
	switch(side)
	{
	case 0:
		x = 1.f; y = -A(j, res); z = A(i, res);
		di = y; dj = z;
		break;
	case 1:
		x = -1.f; y = -A(j, res); z = A(i, res);
		di = y; dj = z;
		break;
	case 2:
		x = A(i, res); y = 1.f; z = A(j, res);
		di = x; dj = z;
		break;
	case 3:
		x = A(i, res); y = -1.f; z = -A(j, res);
		di = x; dj = z;
		break;
	case 4:
		x = A(i, res); y = -A(j, res); z = 1.f;
		di = x; dj = y;
		break;
	case 5:
		x = -A(i, res); y = -A(j, res); z = -1.f;
		di = x; dj = y;
		break;
	default:
		assert(0);
		break;
	}
	solidAngle = di * di + dj * dj + 1.f;
	solidAngle = 1.f / (solidAngle * sqrtf(solidAngle));
	return Normalize(Vec3(x, y, z));
}

//Light offset to light direction: given a (trans vector or light tex image) memory offset, return corresponding light dir
 Vec3 CCubeMap::Offset2Dir(int side, int offset, int res)
{
	//[0,31] -> (-1,1)
#define A(i, res) ((i + 0.5f) * 2.f / res - 1.f)

	int i = offset % res;
	int j = offset / res;
	Vec3 w(0,0,0);

	switch(side)
	{
	case 0:
		w = Vec3(1.f, -A(j, res), A(i, res));
		break;
	case 1:
		w = Vec3(-1.f, -A(j, res), A(i, res));
		break;
	case 2:
		w = Vec3(A(i, res), 1.f, A(j, res));
		break;
	case 3:
		w = Vec3(A(i, res), -1.f, -A(j, res));
		break;
	case 4:
		w = Vec3(A(i, res), -A(j, res), 1.f);
		break;
	case 5:
		w = Vec3(-A(i, res), -A(j, res), -1.f);
		break;
	default:
		//("err!");
		break;
	}
	w.Normalize();
	return w;
}


//given an side, return a LookAt center and up with respect to the eye. Used for visib.
//due to the atlas's intrinsic order, Right(side0)'s s-t single is not right-hand, so can't get a atlas-order visib map side, need flip s after readPixels!!!
void CCubeMap::EyeSide2CenterUp(Vec3* eye, int side, Vec3* center, Vec3* up)
{
	assert(side >= 0 && side<= 5);
	switch(side)
	{
	case 0://right
		*center = *eye + Vec3(1,0,0);
		*up = Vec3(0.f, -1.f, 0.f);
		break;
	case 1://left
		*center = *eye + Vec3(-1,0,0);
		*up = Vec3(0.f, -1.f, 0.f);
		break;
	case 2://up
		*center = *eye + Vec3(0,1,0);
		*up = Vec3(0.f, 0.f, 1.f);
		break;
	case 3://look down
		*center = *eye + Vec3(0,-1,0);
		*up = Vec3(0.f, 0.f, -1.f);
		break;
	case 4://back!
		*center = *eye + Vec3(0,0,1);
		*up = Vec3(0.f, -1.f, 0.f);
		break;
	case 5://front
		*center = *eye + Vec3(0,0,-1);
		*up = Vec3(0.f, -1.f, 0.f);
		break;
	default:
		break;
	}
}

//only used for LoadPFM, the input .pfm cross is 4*3 blocked, middle col and 2nd(from image view) row has contents only.
int CCubeMap::PFMDispatchFaces(int iCode, int jCode)
{
	if(iCode == 1)
	{
		switch(jCode)
		{
		case 0:
			return 1;
			break;
		case 1:
			return 5;
			break;
		case 2:
			return 0;
		    break;
		case 3:
			return 4;
		    break;
		default:
			return -1;
		    break;
		}
	}
	else if (jCode == 1)
	{
		switch(iCode)
		{
		case 0:
			return 3;
			break;
		case 2:
			return 2;
		    break;
		default:
			return -1;
		    break;
		}
	}
	else return -1;
}

//load pfm of anysize into m_pCube.
bool CCubeMap::LoadPFM(const char *fn)
{
	using namespace std;
	ifstream input(fn, ios::binary);
	if(!input.is_open())
		return false;	

	char sHeader[513];
	input >> sHeader;
	if( !(sHeader[0] == 'P' && sHeader[1] == 'F') && 
		!(sHeader[0] == 'p' && sHeader[1] == 'f') )
		return false;	
	unsigned int pfmW = 0, pfmH = 0, subImgW = 0, subImgH = 0;
	input >> pfmW >> pfmH; //pfmW: width, 3blocks pfmH: height, 4 blocks
	if(pfmW % 3 != 0 || pfmH % 4 != 0)	
		return false;
	subImgW = pfmW / 3; //each block's width
	subImgH = pfmH / 4; //each block's height
	int pfmVec3Cnt = pfmW * pfmH; 
	int subImgVec3Cnt = subImgW * subImgH;
	float ByteOrder = 0;
	input >> ByteOrder;//neglect

	Vec3* pRawData = new Vec3[pfmVec3Cnt];

	if(m_pCube != NULL)
		delete [] m_pCube;
	m_pCube = new Vec3[subImgVec3Cnt * 6];
	
	input.read((char*)pRawData, 1);
	input.read((char *) pRawData,  pfmVec3Cnt * sizeof(Vec3));
	
	//each face tracker
	Vec3* pFacesCur[6];
	for(int i = 0; i < 6; ++i)
	{
		pFacesCur[i] = m_pCube + i * subImgW * subImgH;
	}

	Vec3* pCur = pRawData;
	int subImgIdx = -1;
	for(int j = (int)pfmH - 1; j >= 0; --j)
	{ 
		for(unsigned int i = 0; i < pfmW; ++i)
		{
			if((subImgIdx = PFMDispatchFaces(i / subImgW, j / subImgH)) >= 0)
			{
				*pFacesCur[subImgIdx] = *pCur;
				++pFacesCur[subImgIdx];
			}
			++pCur;
		}
	}

	//rewind each face
	for(int i = 0; i < 6; ++i)
	{
		pFacesCur[i] = m_pCube + i * subImgW * subImgH;
	}

	//to repack memory into glTexImage-compatible layout. refer to the atlas in fileheader
	Transpose(pFacesCur[0], subImgW);
	Flipud(pFacesCur[0], subImgW, subImgH);

	Transpose(pFacesCur[1], subImgW);
	Flipud(pFacesCur[1], subImgW, subImgH);

	Flipud(pFacesCur[2], subImgW, subImgH);
	Transpose(pFacesCur[2], subImgW);

	Flipud(pFacesCur[3], subImgW, subImgH);
	Transpose(pFacesCur[3], subImgW);

	Transpose(pFacesCur[4], subImgW);
	Flipud(pFacesCur[4], subImgW, subImgH);

	Transpose(pFacesCur[5], subImgW);
	Flipud(pFacesCur[5], subImgW, subImgH);


	m_nFaceWidth = subImgW;
	m_nFaceHeight = subImgH;
	delete [] pRawData;


	////for test 
	//CImage img;
	//img.Create( subImgW , subImgH, 24);
	//for(int j = 0; j < subImgH; ++j)
	//	for(int i = 0; i < subImgW; ++i)
	//	{
	//		img.SetPixelRGB(i, j, i, j, 0);
	//	}
	//	img.Save("TEX.jpg");
	//	img.Destroy();


	//	//for test 
	//	if(m_bLBDown)
	//	{
	//		for(int side = 0; side < 6; side++)
	//		{
	//			CString sss = "CurLightCoef";
	//			char strSide[2];
	//			_itoa(side, strSide, 10);
	//			sss = sss + strSide + ".pfm";
	//			m_cubeMap.WritePFMRaw(sss, &m_curLightCoefs[side * m_cubeRes * m_cubeRes], m_cubeRes, m_cubeRes);
	//		}
	//	}

	////for test load pfm
	//CImage img;
	//char strSide[2];
	//CString m_visibFilename = "cubeside";
	//for(int side = 0; side < 6; side++)
	//{
	//	img.Create(subImgW, subImgH, 24);

	//	byte* pBits =(byte*) img.GetBits();
	//	int pitch = img.GetPitch();
	//	byte* pSrc = pBits;
	//	int deb = 255;
	//	for(int j = 0; j < subImgH; j++)
	//		for(int i = 0; i < subImgW; i++)
	//		{
	//			pSrc = pBits + (subImgW - 1 - j) * pitch + i * 3; //render the image from lower-left, same as that in vpVertVisbs
	//			Vec3* pV = pFacesCur[side] + j * subImgW + i;
	//			*(pSrc + 2) = (pV->x) * deb;
	//			*(pSrc + 1) = (pV->y) * deb;
	//			*(pSrc    ) = (pV->z) * deb;
	//		}

	//	_itoa(side, strSide, 10);
	//	img.Save(m_visibFilename + strSide + ".jpg");
	//	img.Destroy();
	//}

/////////////////////////////////////////////////////////////////////////down sample
	if(m_pDownCube)
		delete[]m_pDownCube;
	m_pDownCube = new Vec3[m_downRes * m_downRes * 6];

	for(int side = 0; side < 6; side++)
	{
		SimpleDownSample(pFacesCur[side], m_nFaceWidth, m_nFaceHeight, 
			m_pDownCube + side * m_downRes * m_downRes, m_downRes, m_downRes);
	}		

	////for test 
	//for(int side = 0; side < 6; side++)
	//{
	//	CString sss = "CurLight";
	//	char strSide[2];
	//	_itoa(side, strSide, 10);
	//	sss = sss + strSide + ".pfm";
	//	WritePFMRaw(sss, &m_pDownCube[side * m_downRes * m_downRes], m_downRes, m_downRes);
	//}

	for(int side = 0; side < 6; side++)
	{		
		for(int i = 0; i < 2; ++i)
			Gaussian3X3Blur(m_pDownCube + side * m_downRes * m_downRes, m_downRes, m_downRes);
	}		

	////for test 
	//for(int side = 0; side < 6; side++)
	//{
	//	CString sss = "gaussianedDown";
	//	char strSide[2];
	//	_itoa(side, strSide, 10);
	//	sss = sss + strSide + ".pfm";
	//	WritePFMRaw(sss, &m_pDownCube[side * m_downRes * m_downRes], m_downRes, m_downRes);
	//}

	//for(int side = 0; side < 6; side++)
	//{
	//	for(int i = 0; i < 4; ++i)
	//		Gaussian3X3Blur(pFacesCur[side], m_nFaceWidth, m_nFaceHeight);
	//}		

	////for test  
	//for(int side = 0; side < 6; side++)
	//{
	//	CString sss = "gaussianedOrig";
	//	char strSide[2];
	//	_itoa(side, strSide, 10);
	//	sss = sss + strSide + ".pfm";
	//	WritePFMRaw(sss, pFacesCur[side], m_nFaceWidth, m_nFaceHeight);
	//}

	////for test down sample
	//CImage img1;
	//char strSide1[2];
	//CString m_visibFilename1 = "DownCubeside";
	//for(int side = 0; side < 6; side++)
	//{
	//	img1.Create(m_downRes, m_downRes, 24);
	//	byte* pBits =(byte*) img1.GetBits();
	//	int pitch = img1.GetPitch();
	//	byte* pSrc = pBits;
	//	int deb = 255;
	//	for(int j = 0; j < m_downRes; j++)
	//		for(int i = 0; i < m_downRes; i++)
	//		{
	//			pSrc = pBits + (m_downRes - 1 - j) * pitch + i * 3; //render the image from lower-left, same as that in vpVertVisbs
	//			Vec3* pV = m_pDownCube + side * m_downRes * m_downRes + j * m_downRes + i;
	//			*(pSrc + 2) = (pV->x) * deb;
	//			*(pSrc + 1) = (pV->y) * deb;
	//			*(pSrc    ) = (pV->z) * deb;
	//		}
	//		_itoa(side, strSide1, 10);
	//		img1.Save(m_visibFilename1 + strSide1 + ".jpg");
	//		img1.Destroy();
	//}
	return true;
}

//note: for gk, 1536_4 is right, 4_1536 is wrong!
//for test use
bool CCubeMap::WritePFMRaw(const char *fn, Vec3* pRawData, int pfmW, int pfmH)
{
	using namespace std;
	ofstream output(fn, ios::binary);
	if(!output.is_open())
		return false;	

	output << 'P'<<'F'<<endl;
	output <<pfmW<<' '<<pfmH<<endl; 
	output <<-1<<endl;

	output.write((char*)pRawData, pfmW * pfmH * sizeof(Vec3));

	return true;
}


//for test use. 
bool CCubeMap::WritePFMRaw(const char *fn, float* pRawData, int pfmW, int pfmH, int rawBpp)
{

	using namespace std;
	ofstream output(fn, ios::binary);
	if(!output.is_open())
		return false;	
	if((rawBpp == 3)||(rawBpp == 4))	//color
		output << 'P'<<'F'<<endl;
	else	//gray
	output << 'P'<<'f'<<endl;
	output <<pfmW<<' '<<pfmH<<endl; 
	output <<-1<<endl;

	float* pCur = pRawData;
	float tmp;
	if(rawBpp == 1)
	{
	for(int i = 0; i < pfmW * pfmH; ++i)
		{
			tmp = (float)*pCur;
			output.write((char*)&tmp, 1 * sizeof(float));
			++pCur;
		}
	}
	else if(rawBpp == 3)
	{
		for(int i = 0; i < pfmW * pfmH * 3; ++i)	//color: 3 floats for each pixel
		{
		tmp = (float)*pCur;
		output.write((char*)&tmp, 1 * sizeof(float));
		++pCur;
		}
	}
	else if(rawBpp == 4)
	{
		for(int i = 0; i < pfmW * pfmH * 4; ++i)	//color: 3 floats for each pixel
		{
			if(i % 4 != 3)	//skip alpha
			{
				tmp = (float)*pCur;
				output.write((char*)&tmp, 1 * sizeof(float));
			}
			++pCur;
		}
	}
	return true;
}

//for test use. test byte, e.g. visibs. only write single channel
bool CCubeMap::WritePFMRaw(const char *fn, byte* pRawData, int pfmW, int pfmH)
{

	using namespace std;
	ofstream output(fn, ios::binary);
	if(!output.is_open())
		return false;	

	output << 'P'<<'f'<<endl;
	output <<pfmW<<' '<<pfmH<<endl; 
	output <<-1<<endl;

	byte* pCur = pRawData;
	float tmp;
	for(int i = 0; i < pfmW * pfmH; ++i)
	{
		tmp = (float)*pCur;
		output.write((char*)&tmp, 1 * sizeof(float));
		++pCur;
	}
	return true;
}


//for test use
bool CCubeMap::WritePFM(const char *fn)
{
	using namespace std;
	ofstream output(fn, ios::binary);
	if(!output.is_open())
		return false;	

	unsigned int subImgW = m_nFaceWidth, subImgH = m_nFaceHeight;
	unsigned int pfmW = subImgW * 3;
	unsigned int pfmH = subImgH * 4;

	output << 'P'<<'F'<<endl;
	output <<pfmW<<' '<<pfmH<<endl; 
	output <<-1<<endl;

	int pfmVec3Cnt = pfmW * pfmH; 

	Vec3* pRawData = new Vec3[pfmVec3Cnt];
	
	Vec3* pCur = pRawData;
	int subImgIdx = -1;
	for(int j = pfmH - 1; j >= 0; --j)
	{ 
		for(unsigned int i = 0; i < pfmW; ++i)
		{
			if((subImgIdx = PFMDispatchFaces(i / subImgW, j / subImgH)) >= 0)
			{
				*pCur = Vec3(.5f, .5f, .5f);
			}
			else
			{
				*pCur = Vec3(0, 0, 0);
			}
			++pCur;
		}
	}

	output.write((char*)pRawData, pfmVec3Cnt * sizeof(Vec3));

	delete [] pRawData;
	
	return true;

}

//for Dir2NearestColor. input: dir, res. ouput: transVector/teximage memory offset and side. Dir should be normalized.
inline int	CCubeMap::Dir2Offset(Vec3 dir, int res, int& side)
{
	//normal(-1~1) to cube's side coord(0~res)
#define C1(x) (int(((x) + 1.f) * res / 2.f - 0.5f)) //deb : bugged, can't pass OpenCubeMap's test. but below is even more bugged in SSAniso demo, although it can pass the test.
//#define C1(x) (int(((x) + 1.f) * res / 2.f))
	int offset = -1;

	float eps = 1.0e-6f;

	if((dir.x > eps) && 
		((dir.y/dir.x)>=-1) && ((dir.y/dir.x)<=1) && 
		((dir.z/dir.x)>=-1) && ((dir.z/dir.x)<=1))//right
	{
		side = 0;
		dir = dir / dir.x; //scale so that dir.x == 1, so that dir can intersect side0
		offset = (res - 1 - C1(dir.y)) * res +  C1(dir.z);
	}
	else if((dir.x < -eps) &&
		((dir.y / dir.x) >= -1) && ((dir.y / dir.x) <= 1) && 
		((dir.z / dir.x) >= -1) && ((dir.z / dir.x) <= 1))//left		
	{
		side = 1;
		dir = dir / (- dir.x);
		offset = (res - 1 - C1(dir.y)) * res + C1(dir.z);
	}
	else if((dir.y > eps) &&
		((dir.x / dir.y) >= -1) && ((dir.x / dir.y) <= 1) && 
		((dir.z / dir.y) >= -1) && ((dir.z / dir.y) <= 1))//up
	{
		side = 2;
		dir = dir / dir.y;
		offset = C1(dir.z) * res + C1(dir.x);
	}
	else if	((dir.y < -eps) &&
		((dir.x / dir.y) >= -1) && ((dir.x / dir.y) <= 1) && 
		((dir.z / dir.y) >= -1) && ((dir.z / dir.y) <= 1))//down
	{
		side = 3;
		dir = dir / (- dir.y);
		offset = (res - 1 - C1(dir.z)) * res + C1(dir.x);
	}
	else if((dir.z > eps) &&
		((dir.x / dir.z) >= -1) && ((dir.x / dir.z) <= 1) && 
		((dir.y / dir.z) >= -1) && ((dir.y / dir.z) <= 1))//back
	{
		side = 4;
		dir = dir / dir.z;
		offset = (res - 1 - C1(dir.y)) * res + C1(dir.x);
	}
	else if((dir.z < -eps) &&
		((dir.x / dir.z) >= -1) && ((dir.x / dir.z) <= 1) && 
		((dir.y / dir.z) >= -1) && ((dir.y / dir.z) <= 1))//front
	{
		side = 5;
		dir = dir / (- dir.z);
		offset = (res - 1 - C1(dir.y)) * res + (res - 1 - C1(dir.x));
	}
	else
	{
		assert(0);
	}
	assert(offset >= 0);
	return offset;
}

//no blur, just find a color from the DownMap. dir should be normalized outside.
Vec3* CCubeMap::Dir2NearestColor(Vec3& dir)
{
	int side = 0; 
	int offset = Dir2Offset(dir, m_downRes, side);
	return m_pDownCube + side * m_downRes * m_downRes + offset;
}

//if !bDownCube, then m_nFaceWidth should be == m_nFaceHeight
inline Vec3* CCubeMap::Dir2NearestColor(Vec3& dir, bool bDownCube)
{
	int side = 0; 
	int offset;
	if(bDownCube)
	{
		offset = Dir2Offset(dir, m_downRes, side);
		return m_pDownCube + side * m_downRes * m_downRes + offset;
	}
	else
	{
		offset = Dir2Offset(dir, m_nFaceWidth, side);	
		return m_pCube + side * m_nFaceWidth * m_nFaceHeight + offset;
	}	
}

void CCubeMap::Clamp(float minValue, float maxValue)
{
	unsigned int nVec3Count = m_nFaceWidth * m_nFaceHeight * 6; 

	for(unsigned int i = 0; i < nVec3Count; i++)
	{
		m_pCube[i].x = (m_pCube[i].x > maxValue)? maxValue: m_pCube[i].x;
		m_pCube[i].x = (m_pCube[i].x < minValue)? minValue: m_pCube[i].x;
		m_pCube[i].y = (m_pCube[i].y > maxValue)? maxValue: m_pCube[i].y;
		m_pCube[i].y = (m_pCube[i].y < minValue)? minValue: m_pCube[i].y;
		m_pCube[i].z = (m_pCube[i].z > maxValue)? maxValue: m_pCube[i].z;
		m_pCube[i].z = (m_pCube[i].z < minValue)? minValue: m_pCube[i].z;
	}
}

//average each (origW/downW)*(origH/downH) block of origW into downImg. Can only handle origSize/downSize==0 case, e.g, 256->32. todo: template.
void	SimpleDownSample(Vec3* origImg, int origW, int origH, Vec3* downImg, int downW, int downH)
{
	assert(origImg && downImg);
	assert((origW % downW == 0) && (origH % downH == 0));
	int blockW = origW / downW;
	int blockH = origH / downH;

	memset(downImg, 0, downW*downH * sizeof(Vec3));

	for(int i = 0; i < origW; ++i)
		for(int j = 0; j < origH; ++j)
		{
			*(downImg + (j/blockH)*downW + (i/blockW)) += *(origImg + j*origW + i);
		}
	for(int i = 0; i < downW * downH; ++i)
		*(downImg + i) = *(downImg + i) / (float)(blockH * blockW);

	//for test: 	
	//Vec3 origImg[] = {Vec3(1,0.1,0.01), Vec3(2,0.2,0.02), Vec3(3,0.3,0.03), Vec3(4, 0.4,0.04)};
	//Vec3* downImg = new Vec3[1];
	//SimpleDownSample(origImg, 2, 2, downImg, 1,1);
}

void	Gaussian3X3Blur(Vec3* pImg, int imgW, int imgH)
{
	//temporarily 3X3 blur
	const float gaussian[3][3] = {{1,2,1},  {2,4,2},{1,2,1}};
	const float coef = 0.0625f;

	//first: allocate a temp image
	Vec3* pFiltered = new Vec3[(imgW - 2) * (imgH - 2)];
	Vec3* pDst = pFiltered;

	for(int j = 1; j < imgH - 1; j++)
		for(int i = 1; i < imgW - 1; i++)
		{
			Vec3 neiborSum(0, 0, 0);
			for( int jj = 0; jj < 3; jj++)
				for(int ii = 0; ii < 3; ii++)
				{
					Vec3* pNeibor = pImg + (j + jj - 1) * imgW + (i + ii - 1);
					neiborSum += *pNeibor * gaussian[ii][jj];
				}
			//neiborSum *=  fabs(sumB * coef);
			*pDst = neiborSum * coef;
			++pDst;
		}

	//rewind pDst
	pDst = pFiltered;
	//return filtered back to m_imgOriginal
	for(int j = 1 ;j < imgH - 1; j++)
		for(int i = 1 ; i< imgW - 1; i++)
		{
			*(pImg + j * imgW + i) = *pDst;
			++pDst;
		}
	delete[] pFiltered;
}

void 	Transpose(Vec3* pMat, int matW) //pMat must be square
{
	Vec3 tmp = Vec3(0,0,0);
	for(int i = 1; i < matW; ++i) //note 1!!
		for(int j = 0; j < i; ++j)//note < i!!
		{
			tmp = *(pMat + j * matW + i);
			*(pMat + j * matW + i) = *(pMat + i * matW + j);
			*(pMat + i * matW + j) = tmp;
		}
}

void	Flipud(Vec3* pMat, int matW, int matH)
{
	Vec3* pTmp = new Vec3[matW];
	for(int i = 0; i < matH / 2; ++i)	//note < h/2!!
	{
		memcpy(pTmp, pMat + i * matW, sizeof(Vec3) * matW);
		memcpy(pMat + i * matW, pMat + (matH - 1 - i) * matW, sizeof(Vec3) * matW);
		memcpy(pMat + (matH - 1 - i) * matW, pTmp, sizeof(Vec3) * matW);
	}
	delete[] pTmp;

	////for test
	//Vec3 origImg[] = {	Vec3(1,0.1,0.01), Vec3(2,0.2,0.02), Vec3(3,0.3,0.03), 
	//	Vec3(4,0.4,0.04), Vec3(5,0.5,0.05), Vec3(6,0.6,0.06), 
	//	Vec3(7,0.7,0.07), Vec3(8,0.8,0.08), Vec3(9,0.9,0.09)};

	//Transpose(origImg, 3, 3);
	//Flipud(origImg, 3, 3);
}
