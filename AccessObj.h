// AccessObj.h: interface for the CAccessObj class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __ACCESSOBJ_H__
#define __ACCESSOBJ_H__

//#include "CVector.h"

#include "assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl\glaux.h>
#include <atlimage.h>
#include "3DMath.h"

#define _TItplNum 8

#define OBJ_NONE (0)		// render with only vertices 
#define OBJ_FLAT (1 << 0)	// render with facet normals 
#define OBJ_SMOOTH (1 << 1)	// render with vertex normals 
#define OBJ_TEXTURE (1 << 2)	// render with texture coords 
#define OBJ_COLOR (1 << 3)	// render with colors 
#define OBJ_MATERIAL (1 << 4)	// render with materials 

#define objMax(a,b)	(((a)>(b))?(a):(b))
#define objMin(a,b)	(((a)<(b))?(a):(b))
#define objAbs(x)	(((x)>0.f)?(x):(-x))

#define Tri(x) (m_pModel->pTriangles[(x)])
class CAccessObj
{
	//////////////////////////////////////////////////////////////////////
	// Local class dfinintion

	// --------------------------------------------------------------------
	// COBJmaterial: defines a material in a model. 
	class COBJmaterial
	{
	public:
		char name[256];		// name of material
		float diffuse[4];	// diffuse component 
		float ambient[4];	// ambient component 
		float specular[4];	// specular component 
		float emissive[4];	// emissive component 
		float shininess[1];	// specular exponent

		// Texture Data
		bool bTextured;
		char sTexture[256];	// name of texture file
		int nWidth, nHeight;
		unsigned char *pTexImage;
		
		char * MaterialName() {return (name);}

		float * DiffuseComponent() {return (diffuse);}
		float * AmbientComponent() {return (ambient);}
		float * SpecularComponent() {return (specular);}
		float * EmissiveComponent() {return (emissive);}
		float * ShininessComponent() {return (shininess);}
		
		COBJmaterial()
		{
			sprintf (name, "default");
			sTexture[0] = '\0';
			diffuse[0] = diffuse[1] = diffuse[2] = diffuse[3] = 1.0f;
			ambient[0] = ambient[1] = ambient[2] = ambient[3] = 0.1f;
			specular[0] = specular[1] = specular[2] = specular[3] = 0.0f;
			emissive[0] = emissive[1] = emissive[2] = emissive[3] = 0.0f;
			shininess[0] = 10;

			nWidth = nHeight = 0;
			pTexImage = NULL;
			bTextured = FALSE;
		}
		
		virtual ~COBJmaterial()
		{
			if (pTexImage != NULL)
				delete [] pTexImage;
			pTexImage = NULL;
		}

		void LoadTexture()
		{
			if (strlen(sTexture) < 5) return;
			// Delete the old image
			if (pTexImage != NULL)
				delete [] pTexImage;
			pTexImage = NULL;
			nWidth = nHeight = 0;
			bTextured = FALSE;

			CImage img;
			if(FAILED(img.Load(sTexture)))
				return;
			nWidth = img.GetWidth();
			nHeight= img.GetHeight();
			pTexImage = new unsigned char[nWidth * nHeight * 3];
			unsigned char* pImg = pTexImage;
			for(int j = 0; j < img.GetWidth(); ++j)
				for(int i = 0; i < img.GetHeight(); ++i)
				{
					COLORREF col = img.GetPixel(i, j);
					*pImg++ = GetRValue(col);
					*pImg++ = GetGValue(col);
					*pImg++ = GetBValue(col);
				}
			//for test img.Save("TEX.bmp");
			img.Destroy();

			// free memory
			bTextured = TRUE;
		}

		//deb use upper of myself
		//void LoadTexture()
		//{
		//	int		srcp, rslx, rsly, index1, index2, pTemp;
		//	int		srcx, srcy, x, y;
		//	float		daltarx, daltary;
		//	unsigned char	*SImageData;
		//	
		//	BITMAPFILEHEADER head;
		//	BITMAPINFOHEADER info;
		//	FILE		*fp;
		//	
		//	if (strlen(sTexture) < 5) return;

		//	// Delete the old image
		//	if (pTexImage != NULL)
		//		delete [] pTexImage;
		//	pTexImage = NULL;
		//	nWidth = nHeight = 0;
		//	bTextured = FALSE;

		//	/*
		//	 * load a image
		//	 */
		//	fp=fopen(sTexture,"rb");
		//	if(fp==NULL)	return;
		//		
		//	fread(&head,sizeof(BITMAPFILEHEADER),1,fp);
		//	fread(&info,sizeof(BITMAPINFOHEADER),1,fp);

		//	srcx=info.biWidth;
		//	srcy=info.biHeight;		
		//	
		//	srcp = srcx * 3;
		//	if(srcp%4!=0)	srcp = srcp + 4 - srcp % 4;
		//	
		//	SImageData = new unsigned char[srcp*srcy];//don't *3!
		//	if (SImageData == NULL)
		//	{
		//		MessageBox(NULL, "Malloc ImageData mamory failure", "Error", MB_OK);
		//		exit(0);
		//	}
		//	fread (SImageData,1,srcp*srcy,fp);
		//	fclose (fp);

		//	// Texture size
		//	rslx = rsly = 1;
		//	while( (rslx<<1) <= srcx ) rslx <<= 1;
		//	while( (rsly<<1) <= srcy ) rsly <<= 1;
		//	
		//	pTexImage = new unsigned char[rslx * rsly * 3];
		//	nWidth = rslx;
		//	nHeight= rsly;
		//	
		//	// Calc. daltar
		//	daltarx = (float)srcx/(float)rslx;
		//	daltary = (float)srcy/(float)rsly;

		//	// map source picture to texture-image
		//	index1 = index2 = 0;
		//	for( y = 0; y < rsly; y++ )
		//	{
		//		index1 = srcp * (int)( daltary*y );
		//		index2 = y * rslx * 3;
		//		pTemp = index1;
		//		for( x = 0; x < rslx; x ++ )
		//		{
		//			pTexImage[index2]   = SImageData[pTemp+2];
		//			pTexImage[index2+1] = SImageData[pTemp+1];
		//			pTexImage[index2+2] = SImageData[pTemp];
		//			index2 += 3;
		//			pTemp = index1 + 3 * (int)((x+1) * daltarx);
		//		}
		//	}

		//	// free memory
		//	delete [] SImageData;
		//	bTextured = TRUE;
		//}
		
		void SetTexture()
		{
			if (!bTextured)
			{
				glDisable(GL_TEXTURE_2D);
				return;
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pTexImage);
/*			unsigned char *pTempBuff = new unsigned char [nWidth * nHeight * 4];
			int ii, i1, i2;
			i1 = i2 = 0;
			for (ii = 0; ii < nWidth * nHeight; ii ++)
			{
				pTempBuff[i1++] = pTexImage[i2++];
				pTempBuff[i1++] = pTexImage[i2++];
				pTempBuff[i1++] = pTexImage[i2++];
				pTempBuff[i1++] = (ii & 0x01)*255;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, 4, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTempBuff);
			delete [] pTempBuff;
*/
			glEnable(GL_TEXTURE_2D);
		}
	};// ------------------------------------------------------------------
public:
	// --------------------------------------------------------------------
	// COBJtriangle: defines a triangle in a model.
	class COBJtriangle
	{
	public:
		unsigned int vindices[3];	// array of triangle vertex indices 
		unsigned int nindices[3];	// array of triangle normal indices 
		unsigned int tindices[3];	// array of triangle texcoord indices
		unsigned int findex;		// index of triangle facet normal 
		unsigned int mindex;		// index of material
		COBJtriangle()
		{
			vindices[0] = vindices[1] = vindices[2] = 
			nindices[0] = nindices[1] = nindices[2] = 
			tindices[0] = tindices[1] = tindices[2] = 0;
		}
		virtual ~COBJtriangle()	{}
	};// ------------------------------------------------------------------
private:
	// --------------------------------------------------------------------
	// COBJgroup: defines a group in a model.
	class COBJgroup
	{
	public:
		char name[256];			// name of this group 
		unsigned int nTriangles;	// number of triangles in this group 
		unsigned int* pTriangles;	// array of triangle indices 
		unsigned int material;		// index to material for group 
		int material_id;
		char texturename[256];
		bool m_bTexture;
		int m_iTextureType;
		float m_fTran_X;
		float m_fTran_Y;
		float m_fTran_Z;
		float m_fScale_X;
		float m_fScale_Y;
		float m_fScale_Z;
		float m_fRotate_X;
		float m_fRotate_Y;
		float m_fRotate_Z;
		class COBJgroup* next;		// pointer to next group in model 
		
		COBJgroup()
		{
			nTriangles = 0;
			pTriangles = NULL;
			next = NULL;
		}
		
		virtual ~COBJgroup()
		{
			if (nTriangles != 0)
				delete [] pTriangles;
			pTriangles = NULL;
			nTriangles = 0;
		}
	};// ------------------------------------------------------------------

	// --------------------------------------------------------------------
	// COBJmodel: defines a model.
	class COBJmodel
	{
	public:
		//yk
		vector<unsigned int> TrianglesIndices;		//merge each tri's 3 vert indices together. used for glDrawElements
		vector<Vec3>		vpVertexNormals;		//since vpNormals is random-order normal list in obj, not vertex-index-order list, we need a new per-vertex normal here
		vector<Vec3>		vpVertexTexcoords;		
		int					nVisibRes;				//each vertex has visibility of 6 directions to 6 faces. Is the resolution of each face, e.g, 32 (6*32*32 0/1s per vertex)

		//MS
		vector<byte>		vpVertInvisibs;			//rather of bool type. Array of visibilities, its size is [nVertices][6 * nVisibRes * nVisibRes]
		vector<Vec3>		visRatios;				//visble samples / all light samples.
		vector<float>		vpECoefs;				//each vertex has a irradiance vector compressed to nECoefNum coefficients,
		vector<short int>	vpECoefIdx;			//together with its unique index in the cube
											//.E file format: onevertE,onevertEIdx,...interleaved
		int					nECoefNum;				//resolution of vpECoefs
		int					nTdCoefNum;				//vpTdcoefs, 2pass
		vector<Vec3>		vpTdCoefs;				//Td vectors in wavelet space, [nv][6k]. todo: spacitily save it.
		vector<short int>   vpTdCoefIdx;

		//MS 2pass
		vector<float>		vpVertAreas;			//per vert area, starts from 0

		//SS
		int					nSubsurfSampleNum;		//16
		int					nT1CoefNum;
		int					nPhaseSVDterm;			//phase function's svd term

		//this time, I don't save visibs, but save visratio for debug and raw T1 for Anis use.
		vector<Vec3>		vpSubsurfVisRatio;		//layout: [nSubsurfSampleNum][nv+1], used for test, and glDrawElements(each level of sample depth).
#if 1
		vector<byte>		vpT1Coefs[_TItplNum];				//T1 vectors in wavelet space, [nv][256].
#else
		vector<float>		vpT1Coefs;				//T1 vectors in wavelet space, [nv][256].
#endif
		vector<Vec3>		vpDm;					//Negative normal Dm for each vertex
		
		//Final color
		vector<Vec3>		vpColors;				//final outgoing radiance. index starts from 1, 3 floats per vert.

		//original
		char		pathname[256];			// path to this model 
		char		mtllibname[256];		// name of the material library 
		unsigned int nVertices;				// number of vertices in model 
		Vec3*		vpVertices;				// array of vertices
		unsigned int nNormals;				// number of normals in model 
		//since vpNormals is random-order normal list in obj, not vertex-index-order list, we need a new per-vertex normal here
		Vec3*		vpNormals;				// array of normals. //yk Note: not actually used! if use texture, then check its relation with vpNormals again!!!
		unsigned int nTexCoords;			// number of texcoords in model 
		Vec3*		vpTexCoords;			// array of texture coordinates 
		unsigned int nFacetnorms;			// number of facetnorms in model 
		Vec3*		vpFacetNorms;			// array of facetnorms 
		unsigned int nTriangles;			// number of triangles in model 
		COBJtriangle* pTriangles;			// array of triangles 
		unsigned int nMaterials;			// number of materials in model 
		COBJmaterial* pMaterials;			// array of materials 
		unsigned int nGroups;				// number of groups in model 
		COBJgroup*	pGroups;				// linked list of groups 
		Vec3		position;				// position of the model 
		
		//yksse
		//SSediting
		int			ntItplNum;
		int			ngItplNum;
		int			pRes; //HG phase function's wi-wo res

		// construction
		COBJmodel()
		{
			nVisibRes = 0;
			pRes = 0;
			nECoefNum = 0;
			nVertices   = 0;
			vpVertices  = NULL;
			nNormals    = 0;
			vpNormals   = NULL;
			nTexCoords  = 0;
			vpTexCoords = NULL;
			nFacetnorms = 0;
			vpFacetNorms= NULL;
			nTriangles  = 0;
			pTriangles  = NULL;
			nMaterials  = 0;
			pMaterials  = NULL;
			nGroups     = 0;
			pGroups     = NULL;
			position    = Vec3(0, 0, 0);
			
			//yk
			nPhaseSVDterm = 0;
			ntItplNum = 0;
			ngItplNum = 0;
		}
		
		// free all memory
		void	Destory()
		{
			COBJgroup *group;
			if (vpVertices)		delete [] vpVertices;
			if (vpNormals)		delete [] vpNormals;
			if (vpTexCoords)	delete [] vpTexCoords;
			if (vpFacetNorms)	delete [] vpFacetNorms;
			if (pTriangles)		delete [] pTriangles;
			if (pMaterials)		delete [] pMaterials;

			while(pGroups)
			{
				group = pGroups;
				pGroups = pGroups->next;
				delete group;
			}
			nECoefNum = 0;
			nVertices    = 0;
			vpVertices   = NULL;
			nNormals     = 0;
			vpNormals    = NULL;
			nTexCoords   = 0;
			vpTexCoords  = NULL;
			nFacetnorms  = 0;
			vpFacetNorms = NULL;
			nTriangles   = 0;
			pTriangles   = NULL;
			nMaterials   = 0;
			pMaterials   = NULL;
			nGroups      = 0;
			pGroups      = NULL;
			position     = Vec3(0, 0, 0);
		}
		
		// destruction
		virtual ~COBJmodel()
		{
			Destory();
		}
	};// ------------------------------------------------------------------

	// --------------------------------------------------------------------
	// A temporal calss
	class OBJnode
	{
	public:
		unsigned int index;
		bool averaged;
		OBJnode* next;

		OBJnode()
		{
			index = 0;
			next = NULL;
		}
		virtual ~OBJnode() {}
	};// ------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Definition of the OBJ R/W class 
///////////////////////////////////////////////////////////////////////////////
public:
	CAccessObj();
	virtual ~CAccessObj();

	COBJmodel *m_pModel;
	
protected:
	Vec3 m_vMax, m_vMin;

	void CalcBoundingBox();
	bool Equal(Vec3 * u, Vec3 * v, float epsilon);
	
	COBJgroup* FindGroup(char* name)
	{
		COBJgroup * group;
		
		assert(m_pModel);
		group = m_pModel->pGroups;
		while(group)
		{
			if (!strcmp(name, group->name))	break;
			group = group->next;
		}
		
		return group;
	}
	
	COBJgroup* AddGroup(char* name)
	{
		COBJgroup* group;
		group = FindGroup(name);
		if (!group)
		{
			group = new COBJgroup;
			sprintf(group->name, "%s", name);
			group->material = 0;
			group->nTriangles = 0;
			group->pTriangles = NULL;
			group->next = m_pModel->pGroups;
			m_pModel->pGroups = group;
			m_pModel->nGroups++;
		}
		
		return group;
	}
	
	unsigned int FindMaterial(char* name);
	char* DirName(char* path);
	void ReadMTL(char* name);
	void WriteMTL(char* modelpath, char* mtllibname);
	void FirstPass(FILE* file);
	void SecondPass(FILE* file);
	void Scale(float scale);
	void Translate(const Vec3& trans);
	void ReverseWinding();
	void FacetNormals();
	void VertexNormals(float angle);
public:
	void LinearTexture();
	void SpheremapTexture();
	void Destory();
	void Boundingbox(Vec3 &vMax, Vec3 &vMin);
	bool LoadOBJ(char* filename);
	void ComputeNormals();
	bool WriteOBJ(char* filename, unsigned int mode);

	//yk
	//scale obj vertices so that the new bbox's size is newBboxsize, i.e., max coords' range is [-newBboxsize/2.f, newBboxsize/2.f]
	void CenterScaleObj(float newBboxSize);
	bool LoadVisibs(char* filename);
	bool LoadECoefs(char* filename);
	bool LoadTd2Coefs(char* filename);	//2pass

	//.srv: [sampleNum][nv+1]
	bool LoadSubsurfVisRatio(char* filename);

	//Load .T1: [nv][CoefNum]
	bool LoadT1Coefs(CString filename);	

	//Load Dm. float [nv+1]
	bool LoadDm(char* filename);

	//calc per vert area, for MS kd tree use
	void CalcVertArea();

	void VertTexcoords();
};

#endif