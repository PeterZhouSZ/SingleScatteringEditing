// PRTView.h : interface of the CPRTView class
//

#pragma once

#include "AccessObj.h"
#include "Cubemap.h"
#include "Wavelet.h"
#include "ArcBall.h"
#include "KdTree.h"
#include "gl\glext.h"
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
//#include <gl\glext.h>
#include <atlimage.h>

#include "cwc\aGLSL.h"
#include "DlgInfo.h"//dlg

#pragma comment( lib, "opengl32.lib")
#pragma comment( lib, "glu32.lib")
#pragma comment( lib, "glaux.lib" )	
//#define _TItplNum 8

class CPRTView : public CView
{
	//signal
	bool	m_bLoadedObj;
	bool	m_bLoadedVisibs;		//including both case: loaded from file, or computed
	bool	m_bLoadedRd;			//Rd(||xi-xo||)
	bool	m_bLoadedTdCoefs;		//Td wavelet coefs.
	bool	m_bLoadedCubeMap;
	bool	m_bLoadedSubsurfVisRatio; //SS sample.
	bool	m_bLoadedT1Coefs;		//T1 wavelet coefs
	

	bool	m_bDrawNormal;			//with little line
	bool	m_bDrawNormalMap;		//with (xyz+1)*127.5
	bool	m_bDrawVisRatio;		//visRatio [0,1]
	bool	m_bDrawMS;				//multiple scattering
	bool	m_bDrawLight;			//Draw additional light

	bool	m_bDrawSS;				//SS iso / aniso
	
	bool    m_bLBDown;
	bool    m_bRBDown;
	bool	m_bMBDown;
	int		m_oldY;					//for zoom

	bool	_DiffuseE;				//MS transport is constant1
	bool	_Fwi1;					//fresnelT of wi is 1
	bool	_Fwo1;					//fresnelT of wo is 1
	bool	_MC;					//single scattering sampling, Monte Carlo sample. otherwise, uniform sample
	bool	m_bMSUncomp;			//Uncompressed
	bool	_HG1;					//SSAniso's HG's hk and gk is const 1
	bool	_Spec;					//add spec to final color
	bool	_Tex;					//tex

	bool	m_bDrawSubsurfVisratio;	//Draw svr, loop m_SSSampleNum indices

	bool	m_bLoadedDm;
	bool	m_bDrawDm;				//draw Dm
	bool	m_bDrawBBox;			//draw Kdtree bounding box 's first 3 levels: r,g,b-> depth1,2,3. depth0 not drawn.
	
	bool	m_bLoadedECoefs;

	//file
	FILE*	m_fTd0;
	FILE*	m_fE0;

	//data
	float	m_objBboxSize;	//scale new size to 10, i.e., max/min Coord is [Bboxsize/2, -Bboxsize/2]
	int		m_cubeRes; //m_accessObj's visibility resolution. also used for cube's resolution.
	
	CAccessObj m_accessObj;//contains transport vectors of each vert
	
	CCubeMap m_cubeMap;
	
	unsigned int m_ECoefNum; //pre-computed transport num

	unsigned int m_useCoefNum;	//light wavelet num, i.e, use coef num.//note: if useCoef != T1/dcoef, then T1/dCoef must be sorted first!!!
	
	CWavelet m_wavelet;
	
	vector<float>	m_curLightCoefs[3]; //dynamically changed light's (lossless) Haared coefs. to save dynamic memcpy, is also used as curlightCube at sampling phase.
	
	float*	m_bufFloatCube;
	Vec3*   m_bufVecCube;

	float	m_MSWeight; //synthetic: MS contributes in 1.
	float	m_SSWeight;

	CKdTree* m_pECoefsTree;
	int		m_leafSampleNum;
	float	m_epsMSTdKd;	//epsilon in Jensen02

	unsigned int m_TdCoefNum; //pre-computed Td transport num


	//Rd
	float	m_yita;
	Vec3*	m_Rd;	//precompute Rd(||xi - xo||)
	Vec3	m_zr;
	Vec3	m_zv;
	Vec3	m_sigmaTr;
	Vec3	m_alphaPrime;
	Vec3	m_sigmaSPrime;
	Vec3	m_sigmaA;
	Vec3	m_sigmaS;				//sigmaS = sigmaS'/(1-g)
	Vec3	m_sigmaT;//				 = m_sigmaA + sigmaS;
	float	m_sigmaTLumi;

	float	m_g;//only used by SSsampleIntrude. mean cosine of the scattering angle. >0 if forward Scattering, < 0 if backward scattering, =0 if isotropic.
	
	//SSIso
	int		m_SSSampleNum; //16
	unsigned int m_T1CoefNum; //pre-computed T1 transport num
	GLuint m_displayListId;
	unsigned int m_nVBOVertices;// Vertex VBO Name
	unsigned int m_nVBOColors;// Vertex VBO Name
	
	//SSAniso
	int		m_phaseSVDterm;	//HG phase function SVD, recovered to g[k][_T6k] * h[k][_T6k]. this is k.
	vector<float>	m_pH;	//HG's hk[svdterm][_T6k]
	int		m_HGres;		//HG's res
	int		m_usePhaseTerm; //render use term
	float	m_specCoef;		//spec 
	GLuint	m_meshTex;
	vector<unsigned char>	m_textureImage;
	GLuint	m_texCube;		
	int		m_texW;
	int		m_texH;
	unsigned char	m_texAlpha;

	//view
	Vec3	m_eye;
	int     m_nHeight;  // Stores the height of the View
	int     m_nWidth;   // Stores the width of the view
	float	m_dFOV;	//for zoom of glupersp
	float	m_oldFOV; //for zoom of MiddleButtonDown
	float	m_colorCoef; //wheel scale outgoing radiance brightness
	float	m_SSCoef;	//wheel+Leftbtn scale light cubemap pfm brightness	

	/* OpenGL variables */
	HGLRC   m_hRC; // Permanent Rendering Context
	HDC     m_hDC; // Private GDI Device Context
	int		m_fillMode;//point/line/fill

	//misc
	CString m_filenameNoSuffix;
	float	m_FPS;
	int		m_nFrames;					// FPS and FPS Counter
	DWORD	m_dwLastFPS;						// Last FPS Check Time
	GLubyte* m_errorString;						//gluErrorString(glGetError()) use
	int		m_svrKeyDownIdx;						//0-15 for 16 Xp, loop for OnKeyDown debug use.
	CString m_folderPath;

	//yksse
	//SSediting
	CDlgInfo* m_pDlgInfo;//dlg
	bool	m_bSSEditAniso;//if true, must triple product L*P*T; else double L*T
	bool	m_bLoadedHGCoefSum;
	Vec3	m_SSEditG;
	int		m_gItplNum;
	bool	_AreaWeightedHaarNoSort;//do area weighted wavelet, don't sort the idx. if want sort, copy to a new func and open the switch
	vector<float>	m_curLightCoefSq[3];
	vector<float>	m_curLightBigCoef[3];
	vector<float>	m_curLightCoefSum[3];
	vector<short int>	m_curLightBigCoefIdx[3]; //short is 16bit,2^16 =64k, far enough for 6144
	vector<float>	m_gArray;
	vector<float>	m_tArray;
	int		m_gIdx1[3], m_gIdx2[3]; //g itpl idx
	float	m_gW1[3], m_gW2[3];		//g itpl weight
	int		m_tIdx1[3], m_tIdx2[3]; //simaT itpl idx
	float	m_tW1[3], m_tW2[3];		//sigmaT itpl weight
	vector<float>	m_pGIt;		//rendering interpolation
	vector<float>	m_pGSumIt;
	vector<float>	m_pTIt;
	vector<byte>	m_pHGCoefSum;
	bool	m_bPreRendering;
	vector<Vec3>	m_offset2DirCache;
protected: // create from serialization only
	CPRTView();
	DECLARE_DYNCREATE(CPRTView)

// Attributes
public:
//	CPRTDoc* GetDocument() const;

// Operations
public:

	BOOL		SetupPixelFormat();

	void		InitGL();

	void		DrawScene();

	void		DrawCube(float cubeSize);

	void		MouseTranslate(UINT nFlags, int x, int y);
		
	void		MouseSpinGlobal(UINT nFlags, int x, int y, int init);
	
	//Rd(||xi-xo||) symmetric matrix
	void		ComputeSaveRd();

	//standing at pVert, lookAt 1 of 6 sides of resolution m_pModel's visibRes, fill the visib map of pVert into visibs..
	int			VisibleLightSide(byte* visibs, Vec3* pVert, int side);
	
	//fresnel transmittance. Fresnel reflectance + transmittance = 1.
	inline float FresnelT(float Cos);

	inline float  FresnelR(float Cos);

	//use m_pDownCube / m_pCube as texture
	void		InitCubeMap();

	//////////////////////////////////////////////////////////////////////////Multiple Scattering
	
	//find Ep by Ev//find E of sample samplePoint, by search tree of SampleIrraTran
	//todo:2pass float*		MSPointIrraTran(Vec3 samplePoint);

	//symmetric matrix, but symmetry is not utilized. input 2 verts' index, ouput the float diffuse reflectance
	Vec3		Rd(int xiIdx, int xoIdx); 
	
	//for 2-pass usage
	Vec3		Rd(float r);

	//dynamically sample light,
	void		CalcCurrentLightCube();
	
	//changing light will call it. only changing view won't call it.
	inline void		CalcLightCoef();

	//Fwo * transportedRadiance
	void		CalcColor(); 

	//Jianwei's strong!
	struct xLess
	{
		xLess(Vec3* pVec) : m_pVec(pVec) { }

		bool operator()(short int vIdx1, short int vIdx2)
		{ return (m_pVec[vIdx1].x < m_pVec[vIdx2].x); }
		Vec3* m_pVec;
	};

	struct yLess
	{
		yLess(Vec3* pVec) : m_pVec(pVec) { }

		bool operator()(short int vIdx1, short int vIdx2)
		{ return (m_pVec[vIdx1].y < m_pVec[vIdx2].y); }
		Vec3* m_pVec;
	};

	struct zLess
	{
		zLess(Vec3* pVec) : m_pVec(pVec) { }
		bool operator()(short int vIdx1, short int vIdx2)
		{ return (m_pVec[vIdx1].z < m_pVec[vIdx2].z); }
		Vec3* m_pVec;
	};

	////C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\include\functional, greater
	//template<class _Ty>
	//struct greater
	//	: public binary_function<_Ty, _Ty, bool>
	//{	// functor for operator>
	//	bool operator()(const _Ty& _Left, const _Ty& _Right) const
	//	{	// apply operator> to operands
	//		return (_Left > _Right);
	//	}
	//};

	void		BuildTree(CKdTree* pt);

	//has alloced pt->pL and pR, now find nth(pt->VIds, DIM(pt->depth)), fill VIndices and Boxes of pL and pR
	void		KdNodeSplit(CKdTree* pt);

	void		CalcTdCoefs(Vec3* oneTd,  Vec3* pVert, CKdTree* m_pECoefsTree);

	//use _T6k pBufOneE to accum all pt's offsprings' ECoefs
	void		RecurAccumETree(CKdTree* pt, float* pBufOneE);

	//accumulate Rd*E to Td, use pBuf as buf so to avoid frequent new/delete
	void		RecurAccumTd(Vec3* pBufOneTd, Vec3* pVert, CKdTree* pt);

	//////////////////////////////////////////////////////////////////////////Single Scattering

	//return sampleNum=16 intrude distances from verts, whose sampling path depth is dm. intrudes should have sampleNum elements!!!
	void		SSSampleIntrude(float* intrudes, float* pdfs, int sampleNum, float dm);

	//input: Xp, side. output: stencil->pSSVis; depth->Si; color->Ni(Norm xi).
	//standing at pXp, which is below surface, lookAt 1 of 6 sides of resolution m_pModel's visibRes, 
	//fill the 2nd visib map of pVert into subsurfvisibs. fill normals of intersected (interpolated) pixel-normals into Norm, fill depths into spPrime.
	int			SubSurfVisibleLightSide(byte* pSSVisOneSide, float* pSiOneSide, Vec3* pNiOneSide, Vec3* pXp, int side, long int& debugXpOutSum);

	//stand at pXp, lookat _T6k dirs, to get vis,si,ni
	void			SubsurfLookat(Vec3* pXp, byte* pSSVisOneVert, float* pSiOneVert, Vec3* pNiOneVert);
	
	void		SubsurfLookatInitGL();

	void		ChangeDrawElements();

	void		SubsurfLookatVBOs();

	void		SubsurfBlend(Vec3* pOneT1, byte* pSSVisOneVert, float* pSiOneVert, Vec3* pNiOneVert, float* pIntrude, float* pPdf);

	//generate 6*res*res wi and also rename it to wo, shape it to a HG(wi,wo) matrix, save to .hg, //so matlab could SVD it and get g[4][6k], h[4][6k]
	void		ComputeSaveHGMat(int res);

	void		ComputeSaveHGCoefSum(const float* gArray, const int gNum);

	//HG(Cos, g)
	inline float HG(float Cos);
	
	//SSediting
	void SetSigmaSR(float newNum);
	void SetSigmaSG(float newNum);
	void SetSigmaSB(float newNum);
	void SetSigmaTR(float newNum);
	void SetSigmaTG(float newNum);
	void SetSigmaTB(float newNum);
	void SetMatByIndex(int idx);//0-39,but36-39 is invalidated
	void SetGR(float newNum);
	void SetGG(float newNum);
	void SetGB(float newNum);
	//void CalcPSumTable();//psum(P,s), 6k/3 floats
	//void CalcPIdxTable();//idx of P[s,M], 6k short ints
	//void CalcTSumTable();//psum(T,s), 6k/3 floats
	//void CalcTIdxTable();//idx of T[s,M], 6k short ints

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CPRTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileOpen();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnInitGL();
	afx_msg void OnDrawNormal();
	afx_msg void OnFillMode();
	afx_msg void OnComputeSaveVisibs();
	afx_msg void OnDrawNormalMap();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDrawVisRatio();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnInitialUpdate();
	afx_msg void OnOpenCubeMap();
	afx_msg void OnUpdateEditDrawNormal(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditDrawNormalMap(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditDrawVisRatio(CCmdUI *pCmdUI);
	afx_msg void OnEditDrawLight();
	afx_msg void OnUpdateEditDrawLight(CCmdUI *pCmdUI);
	afx_msg void OnComputeSaveRd();

	//for each vert, for each xp, calc visib2, accumu to T1, save it. also save subsurf vis ratio(svr)
	afx_msg void OnComputeSaveT1();
	afx_msg void OnDiffuseE();
	afx_msg void OnFwi1();
	afx_msg void OnFwo1();
	afx_msg void OnUpdateDiffuseE(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFwi1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFwo1(CCmdUI *pCmdUI);
	afx_msg void OnUseCoefNum32();
	afx_msg void OnUsecoefnum64();
	afx_msg void OnUsecoefnum96();
	afx_msg void OnUsecoefnum128();
	afx_msg void OnUsecoefnum256();
	afx_msg void OnUpdateUsecoefnum32(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum64(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum96(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum128(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum256(CCmdUI *pCmdUI);
	afx_msg void OnUsecoefnum4();
	afx_msg void OnUsecoefnum8();
	afx_msg void OnUsecoefnum16();
	afx_msg void OnUpdateUsecoefnum4(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum8(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUsecoefnum16(CCmdUI *pCmdUI);
	afx_msg void OnDrawSubsurfVisRatio();
	afx_msg void OnUpdateDrawSubsurfVisRatio(CCmdUI *pCmdUI);
	afx_msg void OnComputeSaveDm();	//depth : using a ray tracer by shooting a ray along the negative normal direction and detecting the intersection. used by SSSample integration upper limit

	afx_msg void OnDrawDm();
	afx_msg void OnUpdateDrawDm(CCmdUI *pCmdUI);
	afx_msg void OnMSTrans2Pass();
	afx_msg void OnEditComputeSaveE();
	afx_msg void OnBuildTree();
	afx_msg void OnLoadTd();
	afx_msg void OnLoadT1Coefs();
	afx_msg void OnUsephaseterm4();
	afx_msg void OnUpdateUsephaseterm4(CCmdUI *pCmdUI);
	afx_msg void OnUsephaseterm8();
	afx_msg void OnUpdateUsephaseterm8(CCmdUI *pCmdUI);
	afx_msg void OnUsephaseterm16();
	afx_msg void OnUpdateUsephaseterm16(CCmdUI *pCmdUI);
	afx_msg void OnUsephaseterm32();
	afx_msg void OnUpdateUsephaseterm32(CCmdUI *pCmdUI);
	afx_msg void OnUsephaseterm1();
	afx_msg void OnUpdateUsephaseterm1(CCmdUI *pCmdUI);
	afx_msg void OnSpec();
	afx_msg void OnUpdateSpec(CCmdUI *pCmdUI);
	afx_msg void OnViewDrawtexture();
	afx_msg void OnUpdateViewDrawtexture(CCmdUI *pCmdUI);
	afx_msg void OnFileOpentexture();
	afx_msg void OnUsephaseterm2();
	afx_msg void OnUpdateUsephaseterm2(CCmdUI *pCmdUI);
	afx_msg void OnUsephaseterm3();
	afx_msg void OnUpdateUsephaseterm3(CCmdUI *pCmdUI);
	afx_msg void OnEditLoadvsb();
	afx_msg void OnEditLoadE();
	afx_msg void OnSSEditAniso();
	afx_msg void OnUpdateSSEditAniso(CCmdUI *pCmdUI);
	afx_msg void OnLoadPhaseCoefs();
	afx_msg void OnComputeSaveHGCoefSum();
};



//#ifndef _DEBUG  // debug version in PRTView.cpp
//inline CPRTDoc* CPRTView::GetDocument() const
//   { return reinterpret_cast<CPRTDoc*>(m_pDocument); }
//#endif

