#pragma once
#include "3Dmath.h"
#include <assert.h>
#include <atlimage.h>

/* OpenGL Cube atlas
	y
	A
	|
	/----> x
   /
  z
				  ____________
				 |     Up(2)  |
				 | t   		  |
				 | |          |
				 | |____s     |
	 ____________|____________|________________________
	|	   s____ |		s____ |	____s	   |	 s____ |
	|		   | |			| ||		   |		 | |		
	|		   | |          | ||           |		 | |
	|Left(1)   t |Front(5)	t |t  Right(0) |Back(4)	 t |
	|____________|____________|____________|___________|
				 |    Down(3) |
				 |t           |
			 	 ||           |
				 ||___s       |
				 |____________|

light env map is drawn in this way, and vertex's transport vector is also arranged in this way.
*/


class CCubeMap
{
public:
	CCubeMap(void);
	~CCubeMap(void);

	//return normalized light dir by lower-left offset of the side
	 Vec3		Offset2Dir(int side, int offset, int res);

	//Light offset to light direction: given a (trans vector or light tex image) memory offset, return corresponding light dir, and its solid angle
	 Vec3		Offset2Dir(int side, int offset, int res, float& solidAngle);

	//input:eye, side. out:center,up. Used by VisibLightSide
	void		EyeSide2CenterUp(Vec3* eye, int side, Vec3* center, Vec3* up);

	//only used for LoadPFM, the input .pfm cross is 4*3 blocked, middle col and 2nd(from image view) row has contents only.
	int	PFMDispatchFaces(int iCode, int jCode);

	//load pfm of anysize into m_pCube.
	bool		LoadPFM(const char *fn);
	
	//for test use
	bool		WritePFM(const char *fn);

	//for test use
	bool		WritePFMRaw(const char *fn, Vec3* pRaw, int w, int h);

	//for test use. test byte, e.g. visibs. only write single channel
	bool		WritePFMRaw(const char *fn, byte* pRawData, int pfmW, int pfmH);

	//for test use. 
	bool		WritePFMRaw(const char *fn, float* pRawData, int pfmW, int pfmH, int rawBpp = 1);

	//clamp each coord of each m_pCube element to [min,max]
	void		Clamp(float minValue, float maxValue);

	//for Dir2NearestColor. input: dir, res. ouput: transVector/teximage memory offset and side. dir should be normalized
	inline int	Dir2Offset(Vec3 dir, int res, int& side); //don't use (Vec3 dir..)!!! would change dir~~~~!~!!

	//no blur, just find a color from the DownMap. dir should be normalized outside.
	Vec3*		Dir2NearestColor(Vec3& dir);

	 //input: dir,res. output: transvector/teximage mem 4 offsets and 4 weights. dir should be normed and is changed
	inline void	Dir2CubeOffsetBilinear(Vec3 dir, const int& res, int* ofWo, float* wWo);

	////no blur, just find a color from the DownMap. dir should be normalized outside.
	//Vec3*		Dir2BinlinearColor(Vec3& dir);

	//if !bDownCube, then m_nFaceWidth should be == m_nFaceHeight
	inline Vec3* Dir2NearestColor(Vec3& dir, bool bDownCube);

	//lightDir w to neibor (16) indices, (the .pfm should be pre-mend-edged), then average the neibors to get a blurred color
	Vec3		LightDir2BlurredColor(Vec3 lightDir);

	Vec3*	m_pCube; //hdr cross, of any size.
	
	Vec3*	m_pDownCube;//blurred, 6*32*32

	int		m_downRes; //down res, 32

	int		m_nFaceWidth;//orig pfm face width
	
	int		m_nFaceHeight;
protected:

};

//average each (origW/downW)*(origH/downH) block of origW into downImg. Can only handle origSize/downSize==0 case, e.g, 256->32. todo: template.
void		SimpleDownSample(Vec3* origImg, int origW, int origH, Vec3* downImg, int downW, int downH);

void		Gaussian3X3Blur(Vec3* origImg, int origW, int origH);

//mat must be square
void 		Transpose(Vec3* pMat, int matWH);

void		Flipud(Vec3* pMat, int matW, int matH);

//maxCoord is 31
inline void N1N2W1W2(const float& x, const int maxCoord, int& n1, int& n2, float& w1, float& w2)
{
	float fCoord = (x + 1.f) / 2.f * (float)(maxCoord); //[-1.f, 1.f] -> [0.f, 31.f]
	n1 = int(fCoord);
	n2 = min(n1 + 1, maxCoord);
	w1 = fCoord - n1;
	w2 = 1.f - w1;
}

//input: dir,res. output: transvector/teximage mem 4 offsets and 4 weights. dir should be normed and is changed
inline void	CCubeMap::Dir2CubeOffsetBilinear(Vec3 dir, const int& res, int* ofWo, float* wWo)
{
	//normal(-1~1) to cube's side coord(0~res)
	//#define C1(x) (int(((x) + 1.f) * res / 2.f - 0.5f)) //deb : bugged, can't pass OpenCubeMap's test. but below is even more bugged in SSAniso demo, although it can pass the test.
	//#define C1(x) (int(((x) + 1.f) * res / 2.f))
	int sideOffset = 0;
	float eps = 1.0e-6f;
	int n1, n2, n3, n4;
	float w1, w2, w3, w4;

	if((dir.x > eps) && 
		((dir.y/dir.x)>=-1) && ((dir.y/dir.x)<=1) && 
		((dir.z/dir.x)>=-1) && ((dir.z/dir.x)<=1))//right
	{
		sideOffset = 0;	//side = 0;
		dir = dir / dir.x; //scale so that dir.x == 1, so that dir can intersect side0
		//nearest:		offset = (res - 1 - C1(dir.y)) * res +  C1(dir.z);
		N1N2W1W2(dir.y, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.z, res-1, n3, n4, w3, w4);
		ofWo[0] = (res - 1 - n1) * res + n3 + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = (res - 1 - n1) * res + n4 + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = (res - 1 - n2) * res + n3 + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = (res - 1 - n2) * res + n4 + sideOffset;
		wWo[3] = w1 * w3;
	}
	else if((dir.x < -eps) &&
		((dir.y / dir.x) >= -1) && ((dir.y / dir.x) <= 1) && 
		((dir.z / dir.x) >= -1) && ((dir.z / dir.x) <= 1))//left		
	{
		sideOffset = 1 * res * res;		//side = 1;
		dir = dir / (- dir.x);
		//nearest:		offset = (res - 1 - C1(dir.y)) * res + C1(dir.z);
		N1N2W1W2(dir.y, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.z, res-1, n3, n4, w3, w4);
		ofWo[0] = (res - 1 - n1) * res + n3 + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = (res - 1 - n1) * res + n4 + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = (res - 1 - n2) * res + n3 + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = (res - 1 - n2) * res + n4 + sideOffset;
		wWo[3] = w1 * w3;
	}
	else if((dir.y > eps) &&
		((dir.x / dir.y) >= -1) && ((dir.x / dir.y) <= 1) && 
		((dir.z / dir.y) >= -1) && ((dir.z / dir.y) <= 1))//up
	{
		sideOffset = 2 * res * res;	//side = 2;
		dir = dir / dir.y;
		//nearest:			offset = C1(dir.z) * res + C1(dir.x);	
		N1N2W1W2(dir.z, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.x, res-1, n3, n4, w3, w4);
		ofWo[0] = n1 * res + n3 + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = n1 * res + n4 + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = n2 * res + n3 + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = n2 * res + n4 + sideOffset;
		wWo[3] = w1 * w3;
	}
	else if	((dir.y < -eps) &&
		((dir.x / dir.y) >= -1) && ((dir.x / dir.y) <= 1) && 
		((dir.z / dir.y) >= -1) && ((dir.z / dir.y) <= 1))//down
	{
		sideOffset = 3 * res * res;		//side = 3;
		dir = dir / (- dir.y);
		//nearest: offset = (res - 1 - C1(dir.z)) * res + C1(dir.x);
		N1N2W1W2(dir.z, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.x, res-1, n3, n4, w3, w4);
		ofWo[0] = (res - 1 - n1) * res + n3 + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = (res - 1 - n1) * res + n4 + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = (res - 1 - n2) * res + n3 + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = (res - 1 - n2) * res + n4 + sideOffset;
		wWo[3] = w1 * w3;
	}
	else if((dir.z > eps) &&
		((dir.x / dir.z) >= -1) && ((dir.x / dir.z) <= 1) && 
		((dir.y / dir.z) >= -1) && ((dir.y / dir.z) <= 1))//back
	{
		sideOffset = 4 * res * res;		//side = 4;
		dir = dir / dir.z;
		//nearest:		offset = (res - 1 - C1(dir.y)) * res + C1(dir.x);
		N1N2W1W2(dir.y, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.x, res-1, n3, n4, w3, w4);
		ofWo[0] = (res - 1 - n1) * res + n3 + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = (res - 1 - n1) * res + n4 + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = (res - 1 - n2) * res + n3 + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = (res - 1 - n2) * res + n4 + sideOffset;
		wWo[3] = w1 * w3;
	}
	else if((dir.z < -eps) &&
		((dir.x / dir.z) >= -1) && ((dir.x / dir.z) <= 1) && 
		((dir.y / dir.z) >= -1) && ((dir.y / dir.z) <= 1))//front
	{
		sideOffset = 5 * res * res;		//side = 5;
		dir = dir / (- dir.z);
		//nearest:	offset = (res - 1 - C1(dir.y)) * res + (res - 1 - C1(dir.x));
		N1N2W1W2(dir.y, res-1, n1, n2, w1, w2);
		N1N2W1W2(dir.x, res-1, n3, n4, w3, w4);
		ofWo[0] = (res - 1 - n1) * res + (res - 1 - n3) + sideOffset;
		wWo[0] = w2 * w4;
		ofWo[1] = (res - 1 - n1) * res + (res - 1 - n4) + sideOffset;
		wWo[1] = w2 * w3;
		ofWo[2] = (res - 1 - n2) * res + (res - 1 - n3) + sideOffset;
		wWo[2] = w1 * w4;
		ofWo[3] = (res - 1 - n2) * res + (res - 1 - n4) + sideOffset;
		wWo[3] = w1 * w3;
	}
	else
	{
		assert(0);
	}
	assert(sideOffset >= 0);
}


