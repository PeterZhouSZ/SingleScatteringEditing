// PRTView.cpp : implementation of the CPRTView class
//

//#define NOMINMAX

#include "stdafx.h"

//FBO
//#include <stdio.h>
//#include <assert.h>
//#include <stdlib.h>
#include "framebufferObject.h"
#include "renderbuffer.h"
#include "glErrorUtil.h"
#include <GL/glut.h>
//
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <iostream>
#include <fstream>
#include <string>

#include "PRT.h"
#include "PRTDoc.h"
#include "PRTView.h"
#include ".\prtview.h"

#include "cwc\aGLSL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Matrix4fT   g_modelTransform = {  1.0f,  0.0f,  0.0f,  0.0f,	0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,   0.0f,  0.0f,  0.0f,  1.0f };
Matrix3fT   g_eyeLastRot = {  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f };
Matrix3fT   g_eyeThisRot = {  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f };
ArcBallT    g_eyeArcBall(640.0f, 480.0f);

Matrix4fT   g_envTransform = {  1.0f,  0.0f,  0.0f,  0.0f,	0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,   0.0f,  0.0f,  0.0f,  1.0f };
Matrix3fT   g_envLastRot = {  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f };
Matrix3fT   g_envThisRot = {  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f };
ArcBallT    g_envArcBall(640.0f, 480.0f);

typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);
// VBO Extension Function Pointers
//PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					// VBO Name Generation Procedure
//PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					// VBO Bind Procedure
//PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					// VBO Data Loading Procedure
//PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;			// VBO Deletion Procedure

#define _GRes 32
#define _G1k 1024		//16*16
//_G6k should == _T6k!!! Refer to CalcColor().case2: pGSumIt + T6kIdx2SumIdx(i1)...
#define _G6k 6144		//6*16*16 
#define _GLevelNum 5	//log2(16)
#define _GSumNum 2046	//6* (_G1k-1)/3

#define _TRes 32
#define _T1k 1024		//32*32
#define _T6k 6144		//6*32*23
#define _TLevelNum 5	//log2(32)
#define _TSumNum 2046	//6* (_T1k-1)/3

#define _TKeepNum 6144	//GT
#define _LKeepNum 6144  //ykdeb: useless to change to 128 etc: "for(_n6k) 3p;"

#define  StencilVis

#define VERT_FILE1 "vs1.vert.cxx"
#define FRAG_FILE1 "fs1.frag.cxx"
#define VERT_FILE2 "vs2.vert.cxx"
#define FRAG_FILE2 "fs2.frag.cxx"

// CPRTView
IMPLEMENT_DYNCREATE(CPRTView, CView)

BEGIN_MESSAGE_MAP(CPRTView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_EDIT_INITGL, OnInitGL)
	ON_COMMAND(ID_EDIT_DRAWNORMAL, OnDrawNormal)
	ON_COMMAND(ID_EDIT_FILLMODE, OnFillMode)
	ON_COMMAND(ID_EDIT_COMPUTESAVEVISBS, OnComputeSaveVisibs)
	ON_COMMAND(ID_EDIT_DRAWNORMALMAP, OnDrawNormalMap)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_EDIT_DRAWVISRATIO, OnDrawVisRatio)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FILE_OPENCUBEMAP, OnOpenCubeMap)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAWNORMAL, OnUpdateEditDrawNormal)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAWNORMALMAP, OnUpdateEditDrawNormalMap)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAWVISRATIO, OnUpdateEditDrawVisRatio)
	ON_COMMAND(ID_EDIT_DRAWLIGHT, OnEditDrawLight)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAWLIGHT, OnUpdateEditDrawLight)
	ON_COMMAND(ID_EDIT_COMPUTESAVET1, OnComputeSaveT1)
	ON_COMMAND(ID_VIEW_32799, OnDiffuseE)
	ON_COMMAND(ID_VIEW_32796, OnFwi1)
	ON_COMMAND(ID_VIEW_32797, OnFwo1)
	ON_UPDATE_COMMAND_UI(ID_VIEW_32799, OnUpdateDiffuseE)
	ON_UPDATE_COMMAND_UI(ID_VIEW_32796, OnUpdateFwi1)
	ON_UPDATE_COMMAND_UI(ID_VIEW_32797, OnUpdateFwo1)
	ON_COMMAND(ID_Menu, OnDrawSubsurfVisRatio)
	ON_UPDATE_COMMAND_UI(ID_Menu, OnUpdateDrawSubsurfVisRatio)
	ON_COMMAND(ID_EDIT_COMPUTESAVEDM, OnComputeSaveDm)
	ON_COMMAND(ID_EDIT_DRAWDM, OnDrawDm)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAWDM, OnUpdateDrawDm)
	ON_COMMAND(ID_EDIT_MSTRANS2PASS, OnMSTrans2Pass)
	ON_COMMAND(ID_EDIT_COMPUTESAVEE, OnEditComputeSaveE)
	ON_COMMAND(ID_EDIT_BUILDTREE, OnBuildTree)
	ON_COMMAND(ID_EDIT_LOADTD, OnLoadTd)
	ON_COMMAND(ID_EDIT_LOADT1, OnLoadT1Coefs)
	ON_COMMAND(ID_USEPHASETERM_4, OnUsephaseterm4)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_4, OnUpdateUsephaseterm4)
	ON_COMMAND(ID_USEPHASETERM_8, OnUsephaseterm8)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_8, OnUpdateUsephaseterm8)
	ON_COMMAND(ID_USEPHASETERM_16, OnUsephaseterm16)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_16, OnUpdateUsephaseterm16)
	ON_COMMAND(ID_USEPHASETERM_32, OnUsephaseterm32)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_32, OnUpdateUsephaseterm32)
	ON_COMMAND(ID_USEPHASETERM_1, OnUsephaseterm1)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_1, OnUpdateUsephaseterm1)
	ON_COMMAND(ID_VIEW_32828, OnSpec)
	ON_UPDATE_COMMAND_UI(ID_VIEW_32828, OnUpdateSpec)
	ON_COMMAND(ID_VIEW_DRAWTEXTURE, OnViewDrawtexture)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAWTEXTURE, OnUpdateViewDrawtexture)
	ON_COMMAND(ID_FILE_OPENTEXTURE, OnFileOpentexture)
	ON_COMMAND(ID_USEPHASETERM_2, OnUsephaseterm2)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_2, OnUpdateUsephaseterm2)
	ON_COMMAND(ID_USEPHASETERM_3, OnUsephaseterm3)
	ON_UPDATE_COMMAND_UI(ID_USEPHASETERM_3, OnUpdateUsephaseterm3)
	ON_COMMAND(ID_EDIT_LOADVSB, OnEditLoadvsb)
	ON_COMMAND(ID_EDIT_LOADE, OnEditLoadE)
	ON_COMMAND(ID_VIEW_SSEDITANISO, OnSSEditAniso)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SSEDITANISO, OnUpdateSSEditAniso)
	ON_COMMAND(ID_EDIT_LOADPHASECOEFS, OnLoadPhaseCoefs)
	ON_COMMAND(ID_EDIT_COMPUTESAVEHGCOEFSUM, OnComputeSaveHGCoefSum)
END_MESSAGE_MAP()

// CPRTView construction/destruction

CPRTView::CPRTView()
:	  m_bLoadedObj(false)
	, m_bLoadedVisibs(false)
	, m_bLoadedRd(false)
	, m_bDrawNormal(false)
	, m_bDrawNormalMap(false)
	, m_bDrawVisRatio(false)
	, m_fillMode(0)
	, m_cubeRes(32)
	, m_ECoefNum(256)
	, m_bLoadedTdCoefs(false)
	, m_bLoadedCubeMap(false)
	, m_bDrawMS(false)
	, m_colorCoef(1.0f)
	, m_SSCoef(1.0f)
	, m_bDrawLight(false)
	, m_bDrawSS(false)
	, m_SSSampleNum(16)
	, m_bLoadedT1Coefs(false)
	, _DiffuseE(false)
	, _Fwi1(true)
	, _Fwo1(true)
	, m_bMSUncomp(false)
	, m_fTd0(NULL)
	, m_fE0(NULL)
	, m_FPS(0)
	, m_nFrames(0)
	, m_dwLastFPS(0)
	, m_objBboxSize(5)
	, m_svrKeyDownIdx(0)
	, m_bDrawSubsurfVisratio(false)
	, m_yita(1.3f)	//index of relative refraction
	, m_bLoadedDm(false)
	, m_bDrawDm(false)
	, _MC(true)
	, m_useCoefNum(256)
	, m_pECoefsTree(NULL)
	, m_epsMSTdKd(0.02f)	//jiaping's is 0.02f 
	, m_bDrawBBox(false)
	, m_TdCoefNum(256)
	, m_T1CoefNum(_TKeepNum)
	, m_displayListId(0)
	, m_nVBOVertices(0)
	, m_nVBOColors(0)
	, m_phaseSVDterm(4) //can't change to bigger for geforce6800 glsl!
	, _HG1(true)//false
	, m_HGres(16)
	, m_usePhaseTerm(1)
	, _Spec(false)
	, m_specCoef(1.f)
	, _Tex(false)
	, m_texAlpha(64)
	, m_dFOV(50)
	, m_bLoadedECoefs(false)
	, m_bLoadedHGCoefSum(false)
	, m_bSSEditAniso(false)//tru: L*P*T triple product. false: L*T double product.
	, _AreaWeightedHaarNoSort(true)
	{
		m_objBboxSize = 100;//ykdeb: tweety:50, buddha:maybe100
		m_sigmaTLumi = 1.f; //ykdeb!!

		//£¡£¡change fs2.frag.glsl accordingly!!!: m_cubeRes=32,m_HGres = 16, m_SSSampleNum = 8.0;
		m_usePhaseTerm = 1;
		m_useCoefNum = 128;
		m_SSSampleNum = 8;

		m_epsMSTdKd = 0.01f;
		m_leafSampleNum = 1;//!!!!!!16;//4 for t2k, 16 for t31k

		m_eye = Vec3(0, 0, 3 * m_objBboxSize);

		m_colorCoef = 0.000242f;//for building:0.002418f; for pixel_blur_area:3.814930f; for grace_cross: 0.000792

		m_SSCoef = 1.f; //for building: 0.074709f;
		m_MSWeight = 0.2f; //to do: add a scroll bar to modify-on-fly
		m_SSWeight = 1.f;
		m_dFOV = 21.2535f; //in degree
		m_specCoef = 0.08f;
		////////////////////////////////////////////////////////////////////////// bssrdf

		//////whole milk
		//m_sigmaSPrime = Vec3(2.55f, 3.21f, 3.77f);//r,g,b (1-g)*sigmaS, sigmaS is scattering coef, not appeared. .Jensen01 Fig.5
		//m_sigmaA = Vec3(0.0011f, 0.0024f, 0.014f);//absorption coef
	
		//marble
		//m_sigmaSPrime = Vec3(2.19f, 2.62f, 3.f);
		//m_sigmaA = Vec3(0.0021f, 0.0041f, 0.0071f);

		////Wang05's teaser
		//m_sigmaSPrime = Vec3(0.75f, 0.85f, 1.00f);//r,g,b (1-g)*sigmaS, sigmaS is scattering coef, not appeared. .Jensen01 Fig.5
		//m_sigmaA = Vec3(0.02f, 0.04f, 0.07f);//absorption coef

		//m_sigmaS = m_sigmaSPrime / (1 - m_g);				//sigmaS = sigmaS'/(1-g)
		//m_sigmaT = m_sigmaA + m_sigmaS;
		//m_sigmaTLumi = 0.299f * m_sigmaT.x + 0.587f * m_sigmaT.y + 0.114f * m_sigmaT.z;	//sigmaT's luminance: L*A*B color space projection of sigmaT

		//milk(lowfat)
		//m_sigmaS = Vec3(0.9124f, 1.0744f, 1.2492f);
		//m_sigmaT = Vec3(0.9126f, 1.0748f, 1.2500f);

		//m_sigmaA = m_sigmaT - m_sigmaS;
		//m_g = 0.f;
		//m_sigmaSPrime = m_sigmaS * (1 - m_g);
		//m_sigmaS = Vec3(1.1124f, 1.2744f, 1.4492f);
		//m_sigmaT = Vec3(1.113f, 1.275f, 1.45f);
		//m_sigmaTLumi = 0.299f * m_sigmaT.x + 0.587f * m_sigmaT.y + 0.114f * m_sigmaT.z;	//sigmaT's luminance: L*A*B color space projection of sigmaT
		
		//float m_zr[3];
		//float m_zv[3];
		//float m_sigmaTr[3];
		//float m_alphaPrime[3];
		//m_g = 0;

		//float Fdr = -1.440f/(m_yita*m_yita) + 0.710f/m_yita + 0.668f + 0.0636f*m_yita;
		
		//Vec3 sigmaTPrime;
		//sigmaTPrime = m_sigmaA + m_sigmaSPrime; //extinction coef
		//m_sigmaTr = SQRTF(DP(m_sigmaA, sigmaTPrime) * 3);	
		//m_alphaPrime = DD(m_sigmaSPrime, sigmaTPrime);

		//m_zr = 1 / sigmaTPrime;
		//m_zv = m_zr * (1 + 4.f/3.f * (1+Fdr)*(1-Fdr));

		//////////////////////////////////////////////////////////////////////////
		m_bLBDown  = false;										
		m_bRBDown = false;	
		m_bMBDown = false;
		m_oldY = 0;//for zoom
		m_oldFOV = m_dFOV;

		for(int rgbIdx = 0; rgbIdx < 3; ++rgbIdx)
		{
			m_curLightCoefs[rgbIdx].resize(_T6k);
			m_curLightCoefSq[rgbIdx].resize(_T6k);
			m_curLightCoefSum[rgbIdx].resize(_TSumNum);
			m_curLightBigCoef[rgbIdx].resize(_LKeepNum);
			m_curLightBigCoefIdx[rgbIdx].resize(_LKeepNum);
		}
		m_cubeMap.m_downRes = m_cubeRes;
		m_textureImage.clear();		
		
		m_pDlgInfo = new CDlgInfo();//dlg
		m_pDlgInfo->Create(IDD_DIALOG_INFO);
		m_pDlgInfo->m_pView = this;
		
		float ts[8] = {0, 0.053569f, 0.16071f, 0.32141f, 0.53569f, 0.80353f, 1.1249f, 1.5f};
		m_tArray.resize(_TItplNum);
		for(int i = 0; i < _TItplNum; ++i)
		{
			m_tArray[i] = ts[i];
		}
		SetMatByIndex(0); //if 0, g>0.9, and gets itpl problem

		//render itpl cache
		m_pGIt.resize(_G6k);
		m_pGSumIt.resize(_G6k);
		m_pTIt.resize(_TKeepNum);
		m_offset2DirCache.resize(_T6k);
		Vec3 *p = &m_offset2DirCache[0];
		for(int sIdx = 0; sIdx < 6; ++sIdx)
			for(int oIdx = 0; oIdx < _T1k; ++oIdx)
				*p++ =  m_cubeMap.Offset2Dir(sIdx, oIdx, m_cubeRes);
}

CPRTView::~CPRTView()
{
	if(m_fTd0)
		fclose(m_fTd0);
	if(m_fE0)
		fclose(m_fE0);

	if(m_pECoefsTree)
	{
		delete m_pECoefsTree;
		m_pECoefsTree = NULL;
	}	

	if(m_pDlgInfo)//dlg
		delete m_pDlgInfo;
}

BOOL CPRTView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CPRTView::OnDraw(CDC* /*pDC*/)
{
}

BOOL CPRTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CPRTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CPRTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

#ifdef _DEBUG
void CPRTView::AssertValid() const
{
	CView::AssertValid();
}

void CPRTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//CPRTDoc* CPRTView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPRTDoc)));
//	return (CPRTDoc*)m_pDocument;
//}
#endif //_DEBUG

int CPRTView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	SetupPixelFormat();
	wglMakeCurrent(NULL,NULL);

	return 0;
}

BOOL CPRTView::SetupPixelFormat()
{
	GLuint PixelFormat;
	static PIXELFORMATDESCRIPTOR pfd= {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size Of This Pixel Format Descriptor
			1,                              // Version Number (?)
			PFD_DRAW_TO_WINDOW |            // Format Must Support Window
			PFD_SUPPORT_OPENGL |            // Format Must Support OpenGL
			PFD_DOUBLEBUFFER,               // Must Support Double Buffering
			PFD_TYPE_RGBA,                  // Request An RGBA Format
			32,                             // Select A 32Bit Color Depth
			0, 0, 0, 0, 0, 0,               // Color Bits Ignored (?)
			0,                              // No Alpha Buffer
			0,                              // Shift Bit Ignored (?)
			0,                              // No Accumulation Buffer
			0, 0, 0, 0,                     // Accumulation Bits Ignored (?)
			24,                             // 24Bit Z-Buffer (Depth Buffer) 
			8,                              // 8 bit stencil buffer
			0,                              // No Auxiliary Buffer (?)
			PFD_MAIN_PLANE,                 // Main Drawing Layer
			0,                              // Reserved (?)
			0, 0, 0                         // Layer Masks Ignored (?)
	};

	m_hDC = ::GetDC(m_hWnd);    // Gets A Device Context For The Window
	PixelFormat = ChoosePixelFormat(m_hDC, &pfd); // Finds The Closest Match To The Pixel Format We Set Above

	if (!PixelFormat)
	{
		::MessageBox(0,"Can't Find A Suitable PixelFormat.","Error",MB_OK|MB_ICONERROR);
		PostQuitMessage(0);
		// This Sends A 'Message' Telling The Program To Quit
		return false ;    // Prevents The Rest Of The Code From Running
	}

	if(!SetPixelFormat(m_hDC,PixelFormat,&pfd))
	{
		::MessageBox(0,"Can't Set The PixelFormat.","Error",MB_OK|MB_ICONERROR);
		PostQuitMessage(0);
		return false;
	}

	m_hRC = wglCreateContext(m_hDC);
	if(!m_hRC)
	{
		::MessageBox(0,"Can't Create A GL Rendering Context.","Error",MB_OK|MB_ICONERROR);
		PostQuitMessage(0);
		return false;
	}

	if(!wglMakeCurrent(m_hDC, m_hRC))
	{
		::MessageBox(0,"Can't activate GLRC.","Error",MB_OK|MB_ICONERROR);
		PostQuitMessage(0);
		return false;
	}
	InitGL();
	return true;
}

void CPRTView::InitGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);

	Matrix3fSetIdentity(&g_eyeLastRot);								// Reset Rotation
	Matrix3fSetIdentity(&g_eyeThisRot);								// Reset Rotation
	Matrix4fSetRotationFromMatrix3f(&g_modelTransform, &g_eyeThisRot);		// Reset Rotation
	Matrix3fSetIdentity(&g_envLastRot);								// Reset Rotation
	Matrix3fSetIdentity(&g_envThisRot);								// Reset Rotation
	Matrix4fSetRotationFromMatrix3f(&g_envTransform, &g_envThisRot);		// Reset Rotation

	// Setup lighting
	GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {100.0};
	GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
	GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	
	ChangeDrawElements();
	CalcLightCoef();
	CalcColor();
}

void CPRTView::ChangeDrawElements()
{
	if(!m_bLoadedObj)
		return;
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	if(m_bLoadedObj)
	{
		if(m_bDrawNormalMap)
		{
			//glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpVertexTexcoords[0]);
			glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpVertexNormals[0]);
		}
		else if(m_bDrawVisRatio && (!m_accessObj.m_pModel->visRatios.empty()))
		{
			glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->visRatios[0]);
		}
		else if(m_bDrawSubsurfVisratio)
		{
			glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpSubsurfVisRatio[m_svrKeyDownIdx * (m_accessObj.m_pModel->nVertices + 1)]);
		}
		else if(m_bDrawDm && m_bLoadedDm)
		{
			glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpDm[0]);
		}
		else if(m_bDrawMS || m_bDrawSS)
		{
			glColorPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpColors[0]);
		}
		else
		{
			glDisableClientState(GL_COLOR_ARRAY);
			glColor3f(1,1,1);
		}
		if(_Tex && (!m_textureImage.empty()) &&(!m_accessObj.m_pModel->vpVertexTexcoords.empty()))
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glDeleteTextures(1, &m_meshTex);
			glGenTextures(1, &m_meshTex);
			glBindTexture(GL_TEXTURE_2D, m_meshTex);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_DECAL);// GL_MODULATE);//
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_texW, m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, &m_textureImage[0]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(3, GL_FLOAT, 0, &m_accessObj.m_pModel->vpVertexTexcoords[0]);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glVertexPointer(3, GL_FLOAT,0, &m_accessObj.m_pModel->vpVertices[0]);
		glNormalPointer(GL_FLOAT, 0, &m_accessObj.m_pModel->vpVertexNormals[0]);
	}
	CheckErrorsGL("Change Draw Elements");
	Invalidate();
}

void CPRTView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	
	m_nHeight= cy;
	m_nWidth = cx;
	g_envArcBall.setBounds((GLfloat)m_nWidth, (GLfloat)m_nHeight);
	g_eyeArcBall.setBounds((GLfloat)m_nWidth, (GLfloat)m_nHeight);
}

void CPRTView::OnFileOpen()
{
	char szFilter[] = "obj Files (*.obj)|*.obj|obj Files (*.obj)|*.obj|All Files (*.*)|*.*||";
	char workdir[128];

	CFileDialog openDlg(TRUE,NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter,
		this);
	CString szFilepath;

	_getcwd( workdir, 128);

	if(openDlg.DoModal() == IDOK)
	{
		BeginWaitCursor();
		m_bLoadedObj = false;
		m_bLoadedVisibs =false;	
		m_bLoadedRd = false;
		m_bLoadedTdCoefs = false;
		m_bDrawVisRatio = false;
		m_bDrawNormalMap = false;
		m_bDrawMS = false;
		m_bDrawSS = false;
		m_bLoadedT1Coefs = false;
		m_bMSUncomp = false;
		m_bDrawSubsurfVisratio = false;
		m_bLoadedSubsurfVisRatio = false;
		m_bLoadedDm = false;
		m_bDrawDm = false;
		m_bDrawLight = false;
		m_bLoadedECoefs = false;
		
		if(m_fTd0)
		{
			fclose(m_fTd0);
			m_fTd0 = NULL;
		}
		if(m_fE0)
		{
			fclose(m_fE0);
			m_fE0 = NULL;
		}

		szFilepath = openDlg.GetPathName();
		if(m_accessObj.LoadOBJ((char*)(LPCSTR)szFilepath))
		{	
			m_accessObj.CenterScaleObj(m_objBboxSize);
			m_accessObj.ComputeNormals();
	
			if(m_accessObj.m_pModel->nMaterials == 0)
			{
				printf("No texture.\n");
				m_accessObj.LinearTexture();
			}
			m_accessObj.VertTexcoords();
			
			//if(m_accessObj.m_pModel->nMaterials > 0)
			//{
			//	for (UINT i = 0; i < m_accessObj.m_pModel->nMaterials; i ++)
			//	{
			//		if(m_accessObj.m_pModel->pMaterials[i].bTextured)
			//		{
			//			m_texW = m_accessObj.m_pModel->pMaterials[i].nWidth;
			//			m_texH = m_accessObj.m_pModel->pMaterials[i].nHeight;
			//			m_textureImage.clear();
			//			m_textureImage.resize(m_texW * m_texH * 4);

			//			unsigned char* pTex = &m_textureImage[0];
			//			unsigned char* pImg = m_accessObj.m_pModel->pMaterials[i].pTexImage;
			//			for(int n = 0; n < m_texW * m_texH * 3; ++n)
			//			{
			//				*pTex = *pImg;
			//				if(n % 3 == 2)
			//				{
			//					++pTex;
			//					*pTex = 255;
			//				}
			//				++pTex;
			//				++pImg;
			//			}	
			//		}
			//	}
			//}
			
			//indices
			m_accessObj.m_pModel->TrianglesIndices.resize(m_accessObj.m_pModel->nTriangles * 3);
			for(unsigned int i = 0; i < m_accessObj.m_pModel->nTriangles; i++)
			{
				for(int j = 0; j < 3; ++j)
					m_accessObj.m_pModel->TrianglesIndices[i * 3 + j] = m_accessObj.m_pModel->pTriangles[i].vindices[j];
			}
			m_accessObj.m_pModel->pRes = _GRes;
			m_accessObj.m_pModel->nVisibRes = m_cubeRes;
			m_accessObj.m_pModel->nECoefNum = m_ECoefNum;
			m_accessObj.m_pModel->nTdCoefNum = m_TdCoefNum;
			m_accessObj.m_pModel->nT1CoefNum = m_T1CoefNum;
			m_accessObj.m_pModel->nSubsurfSampleNum = m_SSSampleNum;
			m_accessObj.m_pModel->nPhaseSVDterm = m_phaseSVDterm;
			m_accessObj.m_pModel->ntItplNum = _TItplNum;
			m_accessObj.m_pModel->ngItplNum = m_gItplNum;
			
			m_filenameNoSuffix = szFilepath.Left((int)strlen(szFilepath) - 4);

			//////////////////////////////////////////////////////////////////////////MS
			//CString m_visibFilename = m_filenameNoSuffix + ".vsb";
			//if(m_accessObj.LoadVisibs((char*)(LPCSTR)m_visibFilename))	
			//{
			//	m_bLoadedVisibs = true;
			//}

			//CString eName = m_filenameNoSuffix + ".E";
			//if(m_accessObj.LoadECoefs((char*)(LPCSTR)eName))	
			//{
			//	m_bLoadedECoefs = true;
			//}

			//CString TdName = m_filenameNoSuffix + ".Td2";
			//if(m_accessObj.LoadTd2Coefs((char*)(LPCSTR)TdName))	
			//{
			//	m_bLoadedTdCoefs = true;
			//}

			if((m_bLoadedTdCoefs || m_bLoadedECoefs) && m_bLoadedCubeMap)
				m_bDrawMS = true;
			//////////////////////////////////////////////////////////////////////////SS
			
			//CString subsurfVisRatioName = m_filenameNoSuffix + ".svr";
			//if(m_accessObj.LoadSubsurfVisRatio((char*)(LPCSTR)subsurfVisRatioName))	
			//{
			//	m_bLoadedSubsurfVisRatio = true;
			//}

			//CString fnDm = m_filenameNoSuffix + ".Dm";
			//if(m_accessObj.LoadDm((char*)(LPCSTR)fnDm))
			//{
			//	m_bLoadedDm = true;
			//}	

			//CString T1Cname = m_filenameNoSuffix + ".T1C";
			//if(m_accessObj.LoadT1Coefs((char*)(LPCSTR)T1Cname))	
			//{
			//	m_bLoadedT1Coefs = true;
			//}

			if(m_bLoadedT1Coefs && m_bLoadedCubeMap)
				m_bDrawSS = true;
			
			//CString HGCname = "HGCoefSum.HGc";
			//if(m_accessObj.LoadHGCoefSum((char*)(LPCSTR)HGCname))	
			//{
			//	m_bLoadedHGCoefSum = true;
			//}
			//////////////////////////////////////////////////////////////////////////
			m_accessObj.m_pModel->vpColors.clear();
			m_accessObj.m_pModel->vpColors.resize(m_accessObj.m_pModel->nVertices + 1);
			m_bLoadedObj = true;
		}
		EndWaitCursor();
		_chdir(workdir);
		InitGL();
	}
	InvalidateRect(NULL,FALSE);

	m_pDlgInfo->ShowWindow(SW_SHOW);//dlg
	m_pDlgInfo->SetWindowPos(&wndTop,850,0,0,0,SWP_NOSIZE);//dlg
	m_pDlgInfo->UpdateData(FALSE);
}

void CPRTView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	DrawScene();
}

void CPRTView::DrawScene()
{	
	wglMakeCurrent(m_hDC,m_hRC);
	glDrawBuffer( GL_BACK );
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(!m_bLoadedObj)
	{
		glFlush();
		SwapBuffers(m_hDC);
		return;
	}
	assert((m_accessObj.m_pModel->nVertices != 0) && (m_accessObj.m_pModel->nTriangles !=0));

	glViewport(0, 0, m_nWidth, m_nHeight);    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(m_dFOV, (GLfloat)m_nWidth / (GLfloat)m_nHeight, 0.1f, 2000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_eye.x, m_eye.y, m_eye.z,
		0.0, 0.0, 0.0,
		0,1,0);
	//view
	glPushMatrix();	
	glMultMatrixf(g_modelTransform.M);
	
	//if(m_nVBOVertices == 1)
	//{
	//	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices);
	//	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		// Set The Vertex Pointer To The Vertex Buffer
	//	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOColors);
	//	glColorPointer( 3, GL_FLOAT, 0, (char *) NULL );		// Set The TexCoord Pointer To The TexCoord Buffer
	////}

	if(m_bLoadedCubeMap && _Tex && (m_meshTex > 0))
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, m_meshTex);
	}
	glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);

	if((m_bDrawNormal || m_bDrawDm ) && (!m_accessObj.m_pModel->vpVertexNormals.empty()))
	{
		glColor3f(1,1,1);
		glBegin(GL_LINES);
		for(unsigned int i = 1; i <= m_accessObj.m_pModel->nVertices; i++)
		{
			Vec3* pVert = &m_accessObj.m_pModel->vpVertices[i];
			Vec3* pNormal = &m_accessObj.m_pModel->vpVertexNormals[i];
			glVertex3f(pVert->x, pVert->y, pVert->z);
			if(m_bLoadedDm && m_bDrawDm)
			{
				float Dm = m_accessObj.m_pModel->vpDm[i].x;
				glVertex3f(pVert->x + Dm * pNormal->x, pVert->y + Dm * pNormal->y, pVert->z + Dm * pNormal->z);
			}
			else
				glVertex3f(pVert->x + 0.2f * pNormal->x, pVert->y + 0.2f * pNormal->y, pVert->z + 0.2f * pNormal->z);
		}
		glEnd();
	}
	//env
	glPushMatrix();	
	glMultMatrixf(g_envTransform.M);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50, (GLfloat)m_nWidth / (GLfloat)m_nHeight, 0.1f, 2000.0f);
	
	if(m_bLoadedCubeMap)
	{
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, m_texCube);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		DrawCube(1000);//cube size
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);//GL_MODULATE);//return to model set
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		CheckErrorsGL("draw cubemap");
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopMatrix();

	GLenum err = glGetError();
	m_errorString = (GLubyte*)gluErrorString(err);
	if(err != 0)
		printf("%s.\n", m_errorString);

#ifdef NDEBUG
	// Get FPS
	if( GetTickCount() - m_dwLastFPS >= 1000 )				// When A Second Has Passed...
	{
		m_FPS = m_nFrames * 1000.f / (GetTickCount() - m_dwLastFPS);						// Save The FPS
		m_dwLastFPS = GetTickCount();					// Update Our Time Variable
		m_nFrames = 0;							// Reset The FPS Counter
		//printf("%d Vertices, %d FPS\n", m_accessObj.m_pModel->nVertices, m_nFPS );
		printf("%.2f FPS\n", m_FPS);
	}
	m_nFrames++;								// Increment Our FPS Counter
#endif

	glFlush();
	SwapBuffers(m_hDC);
}

void CPRTView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;

	m_bLBDown = true;
	g_eyeLastRot = g_eyeThisRot;
	Point2fT MousePt = {(GLfloat)point.x, (GLfloat)point.y};
	g_eyeArcBall.click(&MousePt);

	CView::OnLButtonDown(nFlags, point);
}

void CPRTView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;
	m_bLBDown = false;
	CView::OnLButtonUp(nFlags, point);
}

void CPRTView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;
	m_bRBDown = true;
	g_envLastRot = g_envThisRot;
	Point2fT MousePt = {(GLfloat)point.x, (GLfloat)point.y};
	g_envArcBall.click(&MousePt);
	CView::OnRButtonDown(nFlags, point);
}

void CPRTView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;

	m_bRBDown = false;
	CView::OnRButtonUp(nFlags, point);
}

void CPRTView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;

	m_oldY = point.y;
	m_bMBDown = true;
}

void CPRTView::OnMButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;

	m_oldFOV = m_dFOV;
	m_bMBDown = false;
	CView::OnMButtonUp(nFlags, point);
}

void CPRTView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!m_bLoadedObj)
		return;

	Point2fT MousePt = {(GLfloat)point.x, (GLfloat)point.y};
	Quat4fT     ThisQuat = {0};

	if(m_bMBDown)
	{
		float scale = 1.0f + (float)(m_oldY - point.y) /(float) m_nHeight;
		if(scale > 0.001)
			m_dFOV = m_oldFOV / scale;
		std::cout<<"m_dFOV: "<<m_dFOV<<endl;
		Invalidate(); //calls OnPaint() calls DrawScene()
	}

	if(m_bLBDown)//change view, no need to recalc CalcLightCoef
	{
		g_eyeArcBall.drag(&MousePt, &ThisQuat);				// Update End v And Get Rotation As Quaternion
		Matrix3fSetRotationFromQuat4f(&g_eyeThisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
		Matrix3fMulMatrix3f(&g_eyeThisRot, &g_eyeLastRot);				// Accumulate Last Rotation Into This One
		Matrix4fSetRotationFromMatrix3f(&g_modelTransform, &g_eyeThisRot);	// Set Our Final Transform's Rotation From This One

		CalcColor();//3p
		Invalidate(); //calls OnPaint() calls DrawScene()
	}
	if(m_bRBDown)//change env light
	{
		g_envArcBall.drag(&MousePt, &ThisQuat);				// Update End v And Get Rotation As Quaternion
		Matrix3fSetRotationFromQuat4f(&g_envThisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
		Matrix3fMulMatrix3f(&g_envThisRot, &g_envLastRot);				// Accumulate Last Rotation Into This One
		Matrix4fSetRotationFromMatrix3f(&g_envTransform, &g_envThisRot);	// Set Our Final Transform's Rotation From This One
		
		CalcLightCoef();
		CalcColor();
	}

	Invalidate(); //calls OnPaint() calls DrawScene()
	CView::OnMouseMove(nFlags, point);
}

BOOL CPRTView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CPRTView::OnInitGL()
{
	InitGL();
}

void CPRTView::OnDrawNormal()
{
	m_bDrawNormal = !m_bDrawNormal;
	ChangeDrawElements();
}

void CPRTView::OnFillMode()
{
	m_fillMode = (m_fillMode + 1) % 3;	

	if(m_fillMode == 0)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else if(m_fillMode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if(m_fillMode == 2)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

#if 1 //FBO
void CPRTView::OnComputeSaveVisibs()
{
	if((!m_bLoadedObj) || m_bLoadedVisibs ) //normal for vert Outrude
	{
		printf("!m_bLoadedObj || (m_bLoadedVisibs)!");
		return;
	}
	CString m_visibFilename = m_filenameNoSuffix + ".vsb";
	FILE* f;
	if((f = fopen(((char*)(LPCSTR)m_visibFilename), "wb")) == NULL)
	{
		printf("failed open m_visibFilename \n ");
		return;
	}
	printf("start compute and save T1 Coefs...\n ");
	DWORD dwTmTtlSrt;
	DWORD dwTmTtlEnd;
	dwTmTtlSrt = GetTickCount();
	
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, m_cubeRes, m_cubeRes);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	glDisable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0x0);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp (GL_INCR, GL_INCR, GL_INCR);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1, 0.001f, m_objBboxSize * 2);	//near:intrude[0] is minimal distance that should be covered in [near,far]. 
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//VBO
	// Get Pointers To The GL Functions
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	//Generate And Bind The Vertex Buffer
	glGenBuffersARB( 1, &m_nVBOVertices );					// Get A Valid Name
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			// Bind The Buffer
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), m_accessObj.m_pModel->vpVertices, GL_STATIC_DRAW_ARB );
	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		// Set The Vertex Pointer To The Vertex Buffer

	m_accessObj.m_pModel->vpVertInvisibs.clear();
	m_accessObj.m_pModel->vpVertInvisibs.resize(m_accessObj.m_pModel->nVertices * _T6k);
	//m_accessObj.m_pModel->visRatios.clear();
	//m_accessObj.m_pModel->visRatios.resize(m_accessObj.m_pModel->nVertices  + 1);

	Vec3 VertOutrude = Vec3(0,0,0);
	float outrude = 0.0001f;
	Vec3 Zero3 = Vec3(0,0,0);
	Vec3* pVert = &m_accessObj.m_pModel->vpVertices[0];
	Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[0];
//	Vec3* pDm =  &m_accessObj.m_pModel->vpDm[0];
	Vec3 center = Vec3(0,0,0);
	Vec3 up = Vec3(0,0,0);
	byte* pVisOneVert = &m_accessObj.m_pModel->vpVertInvisibs[0];
	for (unsigned int vIdx = 0; vIdx < m_accessObj.m_pModel->nVertices; ++vIdx)
	{
		++pVert;
		++pNorm;
		VertOutrude = *pVert + *pNorm * outrude;
		for(int side = 0; side < 6; side++)
		{
			glClear(GL_STENCIL_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			m_cubeMap.EyeSide2CenterUp(&VertOutrude, side, &center, &up);
			gluLookAt(VertOutrude.x, VertOutrude.y, VertOutrude.z, center.x, center.y, center.z, up.x, up.y, up.z);
			glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);
			glFlush();
			glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pVisOneVert + side * _T1k); 

			////test
			//CString sss = "testVsb";
			//char strSide[2];
			//_itoa(side, strSide, 10);
			//sss = sss + strSide + ".pfm";
			//m_cubeMap.WritePFMRaw(sss,  &m_accessObj.m_pModel->vpVertInvisibs[vIdx * _T6k + side * _T1k], m_cubeRes, m_cubeRes);
			//CheckErrorsGL();
		}
		Fliplr(pVisOneVert, m_cubeRes, m_cubeRes);//fliplr side 0

		size_t n = fwrite(pVisOneVert, sizeof(byte), _T6k, f);
		assert(n == (size_t)_T6k);
		if(vIdx % 100 == 0)
			printf("visib of vert %d of %d vertices finished. \n", vIdx, m_accessObj.m_pModel->nVertices);

		pVisOneVert += _T6k;
	}
	std::cout<<"Finish T1k coefs ! \n";
	fclose(f);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	if(m_nVBOVertices)
		glDeleteBuffersARB(1, &m_nVBOVertices );						// Free The Memory

	m_bLoadedVisibs = true;
	InitGL();

	////for test
	//unsigned int debugVertIdx = 0; 
	//for(int side = 0; side < 6; side++)
	//{
	//	CString sss = "stencil";
	//	char strSide[2];
	//	_itoa(side, strSide, 10);
	//	sss = sss + strSide + ".pfm";
	//	m_cubeMap.WritePFMRaw(sss, &m_accessObj.m_pModel->vpVertInvisibs[debugVertIdx][side * m_cubeRes * m_cubeRes], m_cubeRes, m_cubeRes);
	//}
}

#else //glVertexPointer
void CPRTView::OnComputeSaveVisibs()
{
	if((!m_bLoadedObj) || m_bLoadedVisibs ) //normal for vert Outrude
	{
		printf("!m_bLoadedObj || (m_bLoadedVisibs)!");
		return;
	}
	CString m_visibFilename = m_filenameNoSuffix + ".vsb";
	FILE* f;
	if((f = fopen(((char*)(LPCSTR)m_visibFilename), "wb")) == NULL)
	{
		printf("failed open m_visibFilename \n ");
		return;
	}

	assert(m_accessObj.m_pModel);

	//###
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT,0, m_accessObj.m_pModel->vpVertices);

	//
#ifdef StencilVis
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, m_cubeRes, m_cubeRes);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0x0);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp (GL_INCR, GL_INCR, GL_INCR);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1, 0.001f, m_objBboxSize * 2);	//near:intrude[0] is minimal distance that should be covered in [near,far]. 
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

#else
	glViewport(0, 0, m_cubeRes, m_cubeRes);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Reset The Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1, 0.01f, m_objBboxSize * 2); //300 should be m_bboxsize*sqrtf(3)
#endif

	m_accessObj.m_pModel->vpVertInvisibs.clear();
	m_accessObj.m_pModel->vpVertInvisibs.resize(m_accessObj.m_pModel->nVertices * _T6k);
	m_accessObj.m_pModel->visRatios.clear();
	m_accessObj.m_pModel->visRatios.resize(m_accessObj.m_pModel->nVertices  + 1);

	Vec3 VertOutrude = Vec3(0,0,0);
	float outrude = 0.0001f;
	Vec3 Zero3 = Vec3(0,0,0);
	int nVisibsPerSide = m_cubeRes * m_cubeRes;
	for(unsigned int i = 0; i < m_accessObj.m_pModel->nVertices; i++)
	{
		Vec3* pVert = &m_accessObj.m_pModel->vpVertices[i + 1]; //obj index starts from 1
		Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[i + 1];
		VertOutrude = *pNorm;
		VertOutrude = VertOutrude * outrude;
		VertOutrude = VertOutrude + *pVert;

		unsigned int nVisibleLightSum = 0;
		for(int side = 0; side < 6; side++)
		{
			nVisibleLightSum += VisibleLightSide(&m_accessObj.m_pModel->vpVertInvisibs[i * _T6k + side * nVisibsPerSide], &VertOutrude, side);
		}
		float ratio = (float)nVisibleLightSum / (float)(m_cubeRes * m_cubeRes * 6);
		m_accessObj.m_pModel->visRatios[i + 1] = Vec3(ratio, ratio, ratio);

		//printf("vert%d's vis ratio is %f\t", i + 1, m_accessObj.m_pModel->visRatios[i + 1].x);
		size_t n = fwrite(&m_accessObj.m_pModel->vpVertInvisibs[i * _T6k], sizeof(byte), _T6k, f);
		assert(n == (size_t)_T6k);

		if(i%100 == 0)
			printf("visib of vert %d of %d vertices finished. \n", i, m_accessObj.m_pModel->nVertices);
	}

	//untested!! flip side0's memory s to res-1-s. 
	for(unsigned int vIdx = 0; vIdx < m_accessObj.m_pModel->nVertices; ++vIdx)
	{
		byte* pVisSide0 = &m_accessObj.m_pModel->vpVertInvisibs[vIdx * _T6k];
		Fliplr(pVisSide0, m_cubeRes, m_cubeRes);
	}

	//dump vis ratios
	size_t n = fwrite(&m_accessObj.m_pModel->visRatios[0], sizeof(Vec3), m_accessObj.m_pModel->nVertices + 1, f);
	assert(n == m_accessObj.m_pModel->nVertices + 1);
	fclose(f);

	m_bLoadedVisibs = true;

#ifdef StencilVis
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif

	InitGL();

	////for test
	//unsigned int debugVertIdx = 0; 
	//for(int side = 0; side < 6; side++)
	//{
	//	CString sss = "stencil";
	//	char strSide[2];
	//	_itoa(side, strSide, 10);
	//	sss = sss + strSide + ".pfm";
	//	m_cubeMap.WritePFMRaw(sss, &m_accessObj.m_pModel->vpVertInvisibs[debugVertIdx][side * m_cubeRes * m_cubeRes], m_cubeRes, m_cubeRes);
	//}
}
#endif

int CPRTView::VisibleLightSide(byte* pVisibsPerSide, Vec3* pVert, int side)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Vec3 center = Vec3(0,0,0);
	Vec3 up = Vec3(0,0,0);
	m_cubeMap.EyeSide2CenterUp(pVert, side, &center, &up);
	gluLookAt(pVert->x, pVert->y, pVert->z,
		center.x, center.y, center.z,
		up.x, up.y, up.z);

	//###
	glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);
	//glBegin(GL_TRIANGLES);
	//for(unsigned int i = 0; i < m_accessObj.m_pModel->nTriangles; i++)
	//{
	//	for(int j = 0; j < 3;j++)
	//	{
	//		Vec3* pVert = &m_accessObj.m_pModel->vpVertices[(m_accessObj.m_pModel->pTriangles[i]).vindices[j]];
	//		glVertex3f(pVert->x, pVert->y, pVert->z);
	//	}
	//}
	//glEnd();

	glFlush();
	glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pVisibsPerSide); 

	//count visRatio
	int visCount = 0;
	GLubyte* pStencil = pVisibsPerSide;
	for(int i = 0; i < m_cubeRes * m_cubeRes; ++i)
	{
		//can see bkgd: visible light
		if(*pStencil == 0)
		{
			visCount++;		
		}
		pStencil ++;
	}
	return visCount;	

//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	Vec3 center = Vec3(0,0,0);
//	Vec3 up = Vec3(0,0,0);
//	m_cubeMap.EyeSide2CenterUp(pVert, side, &center, &up);
//	gluLookAt(pVert->x, pVert->y, pVert->z,
//			center.x, center.y, center.z,
//			up.x, up.y, up.z);
//
//	DrawMesh();
//	glReadBuffer(GL_BACK);
//	
//	glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_RED, GL_UNSIGNED_BYTE, pVisibsPerSide);
//
//	//count visRatio
//	int visCount = 0;
//	GLubyte* pColor = pVisibsPerSide;
//	for(int i = 0; i < m_cubeRes * m_cubeRes; ++i)
//	{
//		//can see bkgd: visible light
//		if(*pColor != 255)
//		{
//			assert(*pColor == 0);
//			visCount++;		
//		}
//		pColor ++;
//	}
//	return visCount;	
}

void CPRTView::OnDrawNormalMap()
{
	if(m_bLoadedObj)
		m_bDrawNormalMap = !m_bDrawNormalMap;
	
	ChangeDrawElements();
}

//a: angle between light and normal
float  CPRTView::FresnelR(float Cos)
{
	//http://scienceworld.wolfram.com/physics/FresnelEquations.html Eq(7),i.e., electric reflectance. PROBLEM: (7)+(8)should =1, but actually not.Unless remove the '-'. ???

	return 1.f - FresnelT(Cos);
    //return  (Cos > 0) ? (sqrtf(m_yita * m_yita - 1 + Cos * Cos) - Cos) / (Cos + sqrtf(m_yita * m_yita - 1 + Cos * Cos)) : 0;//I change cos-... to ...-cos
}

//a: angle between light and normal
float  CPRTView::FresnelT(float Cos)
{
	//http://scienceworld.wolfram.com/physics/FresnelEquations.html Eq(8),i.e., electric transmittance

	//wrong? return  (Cos > 0) ? m_yita * m_yita * Cos / (Cos + 2 * sqrtf(m_yita * m_yita - 1 + Cos * Cos)) : 0;
	return  (Cos > 0) ? 2 * Cos / (Cos + sqrtf(m_yita * m_yita - 1 + Cos * Cos)) : 0;
}

void CPRTView::OnDrawVisRatio()
{
	if(m_bLoadedVisibs && (!m_accessObj.m_pModel->visRatios.empty()))
	{
		m_bDrawVisRatio = !m_bDrawVisRatio;
		ChangeDrawElements();
	}
	else
	{
		printf("!m_bLoadedVisibs or !visRatio\n");
	}
	
}

Vec3 CPRTView::Rd(int xiIdx, int xoIdx) //symmetric matrix, but symmetry is not utilized. input 2 verts' index, ouput the float diffuse reflectance
{
	float r = Distance(m_accessObj.m_pModel->vpVertices[xiIdx + 1], m_accessObj.m_pModel->vpVertices[xoIdx + 1]);

	Vec3 dr, dv, fRd, zero = Vec3(0,0,0);
	
	dr = SQRTF(DP(m_zr, m_zr) + r * r);
	dv = SQRTF(DP(m_zv, m_zv) + r * r);

	fRd = DP(m_alphaPrime / (4.f * PI) , (	DD(DP(m_zr, m_sigmaTr + 1.f / dr, EXPF(DP(- m_sigmaTr, dr))), DP(dr, dr)) +
											DD(DP(m_zv, m_sigmaTr + 1.f / dv, EXPF(DP(- m_sigmaTr, dv))), DP(dv, dv)) ));

	//assert((fRd[0] >= 0)&& (fRd[1] >= 0)&& (fRd[2] >= 0));
	return fRd;
}

Vec3 CPRTView::Rd(float r)
{
	Vec3 dr, dv, fRd, zero = Vec3(0,0,0);

	dr = SQRTF(DP(m_zr, m_zr) + r * r);
	dv = SQRTF(DP(m_zv, m_zv) + r * r);

	fRd = DP(m_alphaPrime / (4.f * PI) , (	DD(DP(m_zr, m_sigmaTr + 1.f / dr, EXPF(DP(- m_sigmaTr, dr))), DP(dr, dr)) +
		DD(DP(m_zv, m_sigmaTr + 1.f / dv, EXPF(DP(- m_sigmaTr, dv))), DP(dv, dv)) ));

	//assert((fRd[0] >= 0)&& (fRd[1] >= 0)&& (fRd[2] >= 0));
	return fRd;
}


inline int SignOfQuadrant(int MIdx, int qx, int qy)
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

//get Coef from Xmat, with s=(level,iIdx,jIdx), M=01/10/11
inline const float& GetCoefSide(int side, const float* Xmat, int level, int iIdx, int jIdx, int MIdx, int res)
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
inline const float GetCoefPSumSide(int side, const float* Xmat, int level, int iIdx, int jIdx, int res)
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
inline void PSum(float* Sum, const float* Coef, int res, int levelNum)
{
	float* pSum = Sum;
	for(int sideIdx = 0; sideIdx < 6; ++sideIdx)
		for(int level = 0; level < levelNum; ++level)
			for(int iIdx = 0; iIdx < (1<<level); ++iIdx)
				for(int jIdx = 0; jIdx < (1<<level); ++jIdx)
				{
					*pSum ++ = GetCoefPSumSide(sideIdx, Coef, level, iIdx, jIdx, res);
				}
}
//
////repack Coef which is in Cubemap idx order, into SqCoef, who's in 3p Idx order
//inline void Repack2SqNoScale(float* SqCoef, const float* Coef, const int res, const int levleNum)
//{
//	float* pSqCoef = SqCoef;
//	for(int sideIdx = 0; sideIdx < 6; ++sideIdx)
//		for(int level = 0; level < levleNum; ++level)
//			for(int iIdx = 0; iIdx < (1<<level); ++iIdx)
//				for(int jIdx = 0; jIdx < (1<<level); ++jIdx)
//				{
//					//res * row + col
//					//M01. col: 2^l +i, row: j
//					*pSqCoef++ = *(Coef + sideIdx*res*res + res * jIdx + (1<<level) + iIdx);
//					//M10. col: i, row: 2^l+j
//					*pSqCoef++ = *(Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + iIdx);
//					//M11. col: 2^l+i, row: 2^l+j
//					*pSqCoef++ = *(Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + (1<<level) + iIdx);
//				}
//}

//repack Coef which is in Cubemap idx order, into SqCoef, who's in 3p Idx order
inline void Repack2SqWithScale(float* SqCoef, const float* Coef, const int res, const int levleNum)
{
	float* pSqCoef = SqCoef;
	for(int sideIdx = 0; sideIdx < 6; ++sideIdx)
	{
		*pSqCoef++ = *(Coef + sideIdx*res*res);
		for(int level = 0; level < levleNum; ++level)
			for(int iIdx = 0; iIdx < (1<<level); ++iIdx)
				for(int jIdx = 0; jIdx < (1<<level); ++jIdx)
				{
					//res * row + col
					//M01. col: 2^l +i, row: j
					*pSqCoef++ = *(Coef + sideIdx*res*res + res * jIdx + (1<<level) + iIdx);
					//M10. col: i, row: 2^l+j
					*pSqCoef++ = *(Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + iIdx);
					//M11. col: 2^l+i, row: 2^l+j
					*pSqCoef++ = *(Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + (1<<level) + iIdx);
				}
	}
}
//
////repack Coef which is in Cubemap idx order, into SqCoef, who's in 3p Idx order
//inline void Repack2SqWithScaleVec3(vector<float>* SqCoef, const Vec3* Coef, const int res, const int levleNum)
//{
//	float* pSqCoefR = &(SqCoef[0])[0];
//	float* pSqCoefG = &(SqCoef[1])[0];
//	float* pSqCoefB = &(SqCoef[2])[0];
//	const Vec3*  pTmp = NULL;
//	for(int sideIdx = 0; sideIdx < 6; ++sideIdx)
//	{
//		*pSqCoefR++ = *(Coef + sideIdx*res*res).x; //scale
//		*pSqCoefG++ = *(Coef + sideIdx*res*res).y;
//		*pSqCoefB++ = *(Coef + sideIdx*res*res).z;
//		for(int level = 0; level < levleNum; ++level)
//			for(int iIdx = 0; iIdx < (1<<level); ++iIdx)
//				for(int jIdx = 0; jIdx < (1<<level); ++jIdx)
//				{
//					//res * row + col
//					//M01. col: 2^l +i, row: j
//					pTmp = Coef + sideIdx*res*res + res * jIdx + (1<<level) + iIdx;
//					*pSqCoefR++ = pTmp->x;
//					*pSqCoefG++ = pTmp->y;
//					*pSqCoefB++ = pTmp->z;
//					//M10. col: i, row: 2^l+j
//					pTmp = Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + iIdx;
//					*pSqCoefR++ = pTmp->x;
//					*pSqCoefG++ = pTmp->y;
//					*pSqCoefB++ = pTmp->z;
//					//M11. col: 2^l+i, row: 2^l+j
//					pTmp = Coef + sideIdx*res*res + res * ((1<<level) + jIdx) + (1<<level) + iIdx;
//					*pSqCoefR++ = pTmp->x;
//					*pSqCoefG++ = pTmp->y;
//					*pSqCoefB++ = pTmp->z;
//				}
//	}
//}
//calls it only when changing light; if only change view, then no need call it.
//find current light in current light rotation, store in m_curLightCoefs
//plan1(simple one):
//cubemap pre-SimpleDownSample(m_CubeMap, m_curLightCoefs);		//to save dynamic memcpy, m_curLightCoefs is also used as curlightCube at sampling phase.
//	for each side, i, j
//		side,i,j -> w', is 6*32*32
//		w = w' * EnvMat.inverse, get orig direction w;
//		get w's orig color: color = LightDir2Color(w)
//	//now we get a new lightmpa, currentLightCube[6*32*32]
//
//plan2(blur one):
//	for each side, i, j  -> w', is 6*256*256
//		w = w' * EnvMat.inverse
//		w -> cubeIndx[1-16]. //the cube should be pre-mend-edged!!
//		color = average(cubeIdx[1-16])
//get a new 6*32*32 currentLightCube
inline void CPRTView::CalcLightCoef()
{
	if(((!m_bLoadedTdCoefs) && (!m_bLoadedECoefs) && (!m_bLoadedT1Coefs)) || (!m_bLoadedCubeMap)) //haven't loaded any coefs
		return;
	
	//now m_curLightCoefs contains 6*32*32 CPoint3Ds
	Vec3 curDir = Vec3(0,0,0);
	Vec3 origDir = Vec3(0,0,0);
	Vec3 tmpV = Vec3(0,0,0);
	float solidAngle = 1.f;
	int tmp = 0;
	for(int side = 0; side < 6; ++side)
	{
		for(int offset = 0; offset < _T1k; ++offset)
		{
			curDir = m_cubeMap.Offset2Dir(side, offset, m_cubeRes, solidAngle);
			//Orig * EnvMat = Cur ==>   Orig = Cur * EnvMat.Inv
			origDir = VecMultMat4Inv(&curDir, &g_envTransform);
			tmpV = *(m_cubeMap.Dir2NearestColor(origDir)) * solidAngle;
			tmp = side * _T1k + offset;
			m_curLightCoefs[0][tmp] = tmpV.x;//r
			m_curLightCoefs[1][tmp] = tmpV.y;//g
			m_curLightCoefs[2][tmp] = tmpV.z;//b
		}
	}
}

inline float DeQuant8(const byte& b, const float& _min, const float& _range)
{
	return _range * static_cast<float>(b) / 255.f + _min;
}
//
////search in pCoefIdx(has idxNum idx) for idx. if found, return its offset in pCoefIdx; otherwise return -1
//inline int SearchIdx(short int* pCoefIdx, int idxNum, int idx)
//{
//	for(int i = 0; i < idxNum; ++i)
//	{
//		if(pCoefIdx[i] == idx)
//		{
//			return i;
//		}
//	}
//	return -1;//this is important!
//}
//
////get Coef from Xmat, with s=(level,iIdx,jIdx), M=01/10/11
//inline const float& GetCoefSide1Channel(int side, int rgbIdx, Vec3* Xmat, int level, int iIdx, int jIdx, int MIdx, int res)
//{
//	if(rgbIdx == 0)//red
//	{
//		switch(MIdx)//res * row + col
//		{
//		case 1://M01. col: 2^l +i, row: j
//			return (*(Xmat + res * jIdx + (1<<level) + iIdx + side*res*res)).x;
//			break;
//		case 2://M10. col: i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + iIdx + side*res*res)).x;
//		case 3://M11. col: 2^l+i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + (1<<level) + iIdx + side*res*res)).x;
//		default:
//			assert(0);
//			break;
//		}
//	}
//	else if(rgbIdx == 1)//green
//	{
//		switch(MIdx)//res * row + col
//		{
//		case 1://M01. col: 2^l +i, row: j
//			return (*(Xmat + res * jIdx + (1<<level) + iIdx + side*res*res)).y;
//			break;
//		case 2://M10. col: i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + iIdx + side*res*res)).y;
//		case 3://M11. col: 2^l+i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + (1<<level) + iIdx + side*res*res)).y;
//		default:
//			assert(0);
//			break;
//		}
//	}
//	else//blue
//	{
//		switch(MIdx)//res * row + col
//		{
//		case 1://M01. col: 2^l +i, row: j
//			return (*(Xmat + res * jIdx + (1<<level) + iIdx + side*res*res)).z;
//			break;
//		case 2://M10. col: i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + iIdx + side*res*res)).z;
//		case 3://M11. col: 2^l+i, row: 2^l+j
//			return (*(Xmat + res * ((1<<level) + jIdx) + (1<<level) + iIdx + side*res*res)).z;
//		default:
//			assert(0);
//			break;
//		}
//	}
//	assert(0);
//	return 0;//never comes here
//}
//
//inline int GetCoefNLIdxside(int side, short int* vBigIdx, int keepNum, int level, int iIdx, int jIdx, int MIdx, int res)
//{
//	int idx = 0;
//	switch(MIdx)//res * row + col
//	{
//	case 1://M01. col: 2^l +i, row: j
//		idx = res * jIdx + (1<<level) + iIdx;
//		break;
//	case 2://M10. col: i, row: 2^l+j
//		idx = res * ((1<<level) + jIdx) + iIdx;
//		break;
//	case 3://M11. col: 2^l+i, row: 2^l+j
//		idx = res * ((1<<level) + jIdx) + (1<<level) + iIdx;
//		break;
//	default:
//		assert(0);
//		break;
//	}
//	idx += side * res * res;
//	for(int i = 0; i < keepNum; ++i)
//	{
//		if(vBigIdx[i] == idx)
//			return i;
//	}
//	return -1;//this is important!
//}
//
//inline int SignOfQuadrant(int MIdx, int qx, int qy)
//{
//	if(MIdx == 1)
//	{
//		if(qx == 0)	return 1;
//		else return -1;
//	}
//	if(MIdx == 2)
//	{
//		if(qy == 0) return 1;//might wrong
//		else return -1;
//	}
//	if(MIdx == 3)
//	{
//		if(qx == 0)
//		{
//			if(qy == 0) return 1;
//			else return -1;
//		}
//		else
//		{
//			if(qy == 0) return -1;
//			else return 1;
//		}
//	}
//	assert(0);
//	return 0;
//}
//
//inline const float GetCoefPSumSide1Channel(int side, int rgbIdx, Vec3* Xmat, int level, int iIdx, int jIdx, int res)
//{
//	if((level == 0) && (iIdx == 0) && (jIdx == 0))
//	{
//		if(rgbIdx == 0)//red
//			return Xmat[side * res * res].x;
//		else if(rgbIdx == 1)//green
//			return Xmat[side * res * res].y;
//		else//blue
//			return Xmat[side * res * res].z;
//	}
//	int ol = level - 1;
//	int oi = iIdx / 2;
//	int oj = jIdx / 2;
//	int qx = iIdx - 2 * oi; 
//	int qy = jIdx - 2 * oj;
//	return GetCoefPSumSide1Channel(side, rgbIdx, Xmat, ol, oi, oj, res) + (1 << ol) *
//		(GetCoefSide1Channel(side, rgbIdx, Xmat, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
//		GetCoefSide1Channel(side, rgbIdx, Xmat, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
//		GetCoefSide1Channel(side, rgbIdx, Xmat, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
//}
//
//inline const byte GetCoefNLside(int side, byte* vBig, short int* vBigIdx, int keepN, int level, int iIdx, int jIdx, int MIdx, int res)
//{
//	int idx = 0;
//	switch(MIdx)//res * row + col
//	{
//	case 1://M01. col: 2^l +i, row: j
//		idx = res * jIdx + (1<<level) + iIdx;
//		break;
//	case 2://M10. col: i, row: 2^l+j
//		idx = res * ((1<<level) + jIdx) + iIdx;
//		break;
//	case 3://M11. col: 2^l+i, row: 2^l+j
//		idx = res * ((1<<level) + jIdx) + (1<<level) + iIdx;
//		break;
//	default:
//		assert(0);
//		break;
//	}
//	idx += side * res * res;
//	for(int i = 0; i < keepN; ++i)
//	{
//		if(vBigIdx[i] == idx)
//			return vBig[i];
//	}
//	return 0;//this is important!
//}
////DANGEROUS!! BUGGED!!! can't use byte. should use float. but then can't dequant. Should first dequant the byte, then can accumulate.
//inline const byte GetCoefPSumNLside(int side, byte* vBig, short int* vBigIdx, int keepN, int level, int iIdx, int jIdx, int res)
//{
//	if((level == 0) && (iIdx == 0) && (jIdx == 0))
//	{
//		int idx = side * res * res;
//		for(int i = 0; i < keepN; ++i)
//		{
//			if(vBigIdx[i] == idx)
//				return vBig[i];
//		}
//		return 0;//this is important!
//	}
//	int ol = level - 1;
//	int oi = iIdx / 2;
//	int oj = jIdx / 2;
//	int qx = iIdx - 2 * oi; 
//	int qy = jIdx - 2 * oj;
//	return GetCoefPSumNLside(side, vBig, vBigIdx, keepN, ol, oi, oj, res) + (1 << ol) *
//		(GetCoefNLside(side, vBig, vBigIdx, keepN, ol, oi, oj, 1, res) * SignOfQuadrant(1, qx, qy) +
//		GetCoefNLside(side, vBig, vBigIdx, keepN, ol, oi, oj, 2, res) * SignOfQuadrant(2, qx, qy) +
//		GetCoefNLside(side, vBig, vBigIdx, keepN, ol, oi, oj, 3, res) * SignOfQuadrant(3, qx, qy));
//}

//
////given i1 in slijm(with scale) idx, find corresponding i2, i3 in the same side
////for paralell, also do another thing: since i1 is in pCoefIdx, fast search if i2,i3 are also in CoefIdx. 
////if yes, return those in pCoef. if not, return nothing(coef2,3 shoud be 0 by defalt input).
//inline void Other2MIdx(const short int i1, short int& i2, short int& i3, int sideSize/*, short int* pCoefIdx, byte* pCoef, byte& Coef2, byte& Coef3*/)
//{
//	int MNo = (i1 % sideSize - 1) % 3; //Mother number: 0, 1 or 2
//	if(MNo == 0)
//	{
//		i2 = i1 + 1;
//		i3 = i1 + 2;
//		return;
//	}
//	else if(MNo == 1)
//	{
//		i2 = i1 - 1;
//		i3 = i1 + 1;
//		return;
//	}
//	else
//	{
//		i2 = i1 - 2;
//		i3 = i1 - 1;
//		return;
//	}
//}

//get other 2 Mother coef Idx, i2, i3; and their corresponding Coef, Ti2, Ti3, if any. may change input i!!!
inline void Other2MIdxTi2Ti3(const short int i1, short int& i2,  short int& i3, float& Ti2, float& Ti3, 
							 const short int* pTIdx, const float* pTIt, unsigned int& i, const int sideSize)
{
	int MNo = (i1 % sideSize - 1) % 3; //Mother number: 0, 1 or 2
	if(MNo == 0)
	{
		i2 = i1 + 1;
		i3 = i1 + 2;
		if(*(pTIdx + i + 1) == i2)	//has M2
		{
			Ti2 = *(pTIt + i + 1);
			++i;
			if(*(pTIdx + i + 1) == i3)	//has M3
			{
				Ti3 = *(pTIt + i + 1);
				++i;
			}
			//else //defalt 0
			//	Ti3 = 0; 
		}
		else
		{
			//Ti2 = 0;
			if(*(pTIdx + i + 1) == i3)	//has M3
			{
				Ti3 = *(pTIt + i + 1);
				++i;
			}
			//else	//defalt 0
			//	Ti3 = 0;
		}
		return;
	}
	else if(MNo == 1)
	{
		i2 = i1 - 1;
		i3 = i1 + 1;
		//Ti2 = 0;
		if(*(pTIdx + i + 1) == i3)	//has M3
		{
			Ti3 = *(pTIt + i + 1);
			++i;
		}
		//else
		//	Ti3 = 0;
		return;
	}
	else
	{
		i2 = i1 - 2;
		i3 = i1 - 1;
		//Ti2 = 0;
		//Ti3 = 0;
		return;
	}
}

//search idx in pSortedIdxArray, if find, fill Coef and return true; else return false
inline bool BiSearch(const short int idx, const short int* pSortedIdxArray, const float* pCoef, const unsigned int arraySize, float& Coef)
{
	int low = 0, high = arraySize - 1, mid;
	while(low <= high)
	{
		mid = (low + high) / 2;
		if(pSortedIdxArray[mid] == idx)
		{
			Coef = pCoef[mid];
			return true;
		}
		else if(pSortedIdxArray[mid] > idx)
			high = mid - 1;
		else
			low = mid + 1;
	}
	Coef = 0;
	return false; //important!
}//log2(n) +1

//T6kIdx: for side +1; for l for i for j.  SumIdx: for side for l for i for j
inline short int T6kIdx2SumIdx(const short int i)
{
	return ((_T1k - 1) / 3) * (i / _T1k) + (i % _T1k -1 ) / 3;
}

//only optimized for _T1k case. can't be used in other res.!!!!
inline void T6kIdx2slij(const short int idx, int& sumSide, int& sumL, int& sumI, int& sumJ)
{
	short int tmpIdx = idx;
	sumSide = tmpIdx / _T1k;
	tmpIdx = tmpIdx % _T1k;
	sumL = log2(tmpIdx /*+ 1*/) / 2; 
	tmpIdx = (tmpIdx - (short int)((1 << (2 * sumL))/* - 1*/)) / 3; //debugged! but no use in GT
	sumI = tmpIdx / (1 << sumL);
	sumJ = tmpIdx % (1 << sumL);
}

//get Idx using s,l,i,j,m. used by TSum. only apply for T1k case. m is 0, 1, or 2.
inline short int T6kSlijm2Idx(int side, int level, int i, int j, int m)
{
	return (short int)(_T1k * side + (1 << (level * 2)) + ((1<<level) * i + j) * 3 + m);
}

//recursive calc psum of T. input: s,l,i,j, in pTIdx and T, whose scale for side s is tScale.
inline float TSum(int side, int level, int iIdx, int jIdx, short int* pTIdx, const float* pT, float tScale, int nT1CoefNum)
{
	if((level == 0) && (iIdx == 0) && (jIdx == 0))
		return tScale;
	int ol = level - 1;
	int oi = iIdx / 2;
	int oj = jIdx / 2;
	int qx = iIdx - 2 * oi; 
	int qy = jIdx - 2 * oj;
	float T01 = 0, T10 = 0, T11 = 0;
	BiSearch(T6kSlijm2Idx(side, ol, oi, oj, 0), pTIdx, pT, nT1CoefNum, T01);
	BiSearch(T6kSlijm2Idx(side, ol, oi, oj, 1), pTIdx, pT, nT1CoefNum, T10);
	BiSearch(T6kSlijm2Idx(side, ol, oi, oj, 2), pTIdx, pT, nT1CoefNum, T11);
	return TSum(side, ol, oi, oj, pTIdx, pT, tScale, nT1CoefNum) + (1 << ol) *
		(	T01 * SignOfQuadrant(1, qx, qy) +
			T10 * SignOfQuadrant(2, qx, qy) +
			T11 * SignOfQuadrant(3, qx, qy)	);
}

//inline void ItplDequant(const float* pIt, const byte* p1, const float& min1, const float& range1, const float& weight1, const byte* p2, const float& min2, const float& range2, const float& weight2, const int len)
//{
//	float* pTmpIt = pIt;
//	byte* pTmp1 = p1;
//	byte* pTmp2 = p2;
//	for(int i = 0; i < len; ++i)
//    {
//		*pTmpIt = weight1 * DeQuant8(pTmp1, min1, range1) + weight2 * DeQuant8(pTmp2, min2, range2);
//		++pTmp1;
//		++pTmp2;
//		++pTmpIt;
//	}
//}
//HG(Cos, g)
inline float HGCosG(const float& Cos, const float& g)
{
	float tmp = 1 + g * g - 2 * g * Cos;
	if(tmp < 1e-5)	//0 or negative
		return 0;
	return (1 - g * g)	/ (tmp * sqrtf(tmp));
//	return (1.f - g * g)	/ ((1.f + g * g - 2.f * g * Cos) * sqrtf(1.f + g * g - 2.f * g * Cos));	//t600 cpu041457
}

//yksse with phase
void CPRTView::CalcColor()
{
	if(((!m_bLoadedTdCoefs) && (!m_bLoadedECoefs) && (!m_bLoadedT1Coefs)) || (!m_bLoadedCubeMap)) //haven't loaded any coefs
		return;
	if(m_bSSEditAniso) 
	{
		float Cos = 0;
		m_accessObj.m_pModel->vpColors[0] = Vec3(0,0,0);
		Vec3 eye = m_eye; //can't directly feed m_eye in below function!
		Vec3 eyeInModelLocal = VecMultMat4Inv(&eye, &g_modelTransform);	//eye * model.inv
		Vec3* pVert = &m_accessObj.m_pModel->vpVertices[0];
		Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[0];
		Vec3* pColor = &m_accessObj.m_pModel->vpColors[0];
		Vec3 Transport;//can be any transport
#if 1	//quantize version
		int oneItplTSize =  _T6k + 2 * sizeof(float);
#else	//unquantize version
		int oneItplTSize =  _T6k;
#endif
		float min1, min2, range1, range2;

		//itpl wi
		for(unsigned int xoIdx = 0; xoIdx < m_accessObj.m_pModel->nVertices; ++xoIdx)
		{
			++pVert;
			++pNorm;
			++pColor;
			Transport = Vec3(0,0,0);
			Vec3 viewLocal = Normalize(eyeInModelLocal - *pVert);
			Cos = Dot(*pNorm, viewLocal);
			if(m_bDrawSS)
			{
				for(int rgbIdx = 0; rgbIdx < 3; ++rgbIdx)
				{
					float integral = 0;

#if 1				//quant version
					//byte dequant version
					byte* p1 = &m_accessObj.m_pModel->vpT1Coefs[m_tIdx1[rgbIdx]][xoIdx * oneItplTSize];
					byte* p2 = &m_accessObj.m_pModel->vpT1Coefs[m_tIdx2[rgbIdx]][xoIdx * oneItplTSize];
					min1 = *(float*)p1;
					p1 += sizeof(float);
					range1 = *(float*)p1;
					p1 += sizeof(float);
					min2 = *(float*)p2;
					p2 += sizeof(float);
					range2 = *(float*)p2;
					p2 += sizeof(float);

					//cache
					Vec3* pTmp = &m_offset2DirCache[0];
					for(int oIdx = 0; oIdx < _T6k; ++oIdx)
					{
						integral += 
							m_curLightCoefs[rgbIdx][oIdx] * 
							HGCosG(Dot(-viewLocal, *pTmp), *m_SSEditG.GetAddress(rgbIdx)) * 
							(m_tW1[rgbIdx] * DeQuant8(*p1, min1, range1) + m_tW2[rgbIdx] * DeQuant8(*p2, min2, range2));
						++pTmp;
						++p1;
						++p2;
					}

#else				//the float version
					float* pT1 = &m_accessObj.m_pModel->vpT1Coefs[xoIdx * oneItplTSize * _TItplNum + m_tIdx1[rgbIdx] * oneItplTSize];
					float* pT2 = pT1 + oneItplTSize;
					Vec3* pTmp = &m_offset2DirCache[0];
					for(int oIdx = 0; oIdx < _T6k; ++oIdx)
					{
						integral += 
							m_curLightCoefs[rgbIdx][oIdx] * 
							HGCosG(Dot(-viewLocal, *pTmp++), *m_SSEditG.GetAddress(rgbIdx)) * 
							(m_tW1[rgbIdx] * (*pT1) + m_tW2[rgbIdx] * (*pT2));
						++pT1;
						++pT2;
					}
#endif
					*(Transport.GetAddress(rgbIdx)) = *m_sigmaS.GetAddress(rgbIdx) * integral * m_SSWeight * m_SSCoef;
				}
			}
			if(_Fwo1)
			{
				*pColor = Transport * m_colorCoef / PI;
			}
			else
			{
				*pColor = Transport * m_colorCoef * FresnelT(Cos) / PI;
			}
			if((m_bDrawSS || m_bDrawMS) && _Spec && (Cos > Epsilon))
			{
				Vec3 reflectLocal = *pNorm * 2.f * Cos - viewLocal;
				Vec3 lightOrig = VecMultMat4Inv(&reflectLocal, &g_envTransform);
				*pColor += *(m_cubeMap.Dir2NearestColor(lightOrig)) * FresnelR(Cos) * m_specCoef;
			}
		}
	}
	else//isotropic: double product L*T
	{
		float Cos = 0;
		m_accessObj.m_pModel->vpColors[0] = Vec3(0,0,0);

		Vec3 eye = m_eye; //can't directly feed m_eye in below function!
		Vec3 eyeInModelLocal = VecMultMat4Inv(&eye, &g_modelTransform);	//eye * model.inv

		Vec3* pVert = &m_accessObj.m_pModel->vpVertices[0];
		Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[0];
		Vec3* pColor = &m_accessObj.m_pModel->vpColors[0];
		Vec3 Transport;//can be any transport

#if 1	//quantize version
		int oneItplTSize =  _T6k + 2 * sizeof(float);
#else	//unquantize version
		int oneItplTSize =  _T6k;
#endif
		float min1, min2, range1, range2;

		for(unsigned int xoIdx = 0; xoIdx < m_accessObj.m_pModel->nVertices; ++xoIdx)
		{
			++pVert;
			++pNorm;
			++pColor;
			Transport = Vec3(0,0,0);
			Vec3 viewLocal = Normalize(eyeInModelLocal - *pVert);
			Cos = Dot(*pNorm, viewLocal);
			if(m_bDrawSS)
			{
				for(int rgbIdx = 0; rgbIdx < 3; ++rgbIdx)
				{
					float integral = 0;

#if 1				//quant version
					//byte dequant version
					byte* p1 = &m_accessObj.m_pModel->vpT1Coefs[m_tIdx1[rgbIdx]][xoIdx * oneItplTSize];
					byte* p2 = &m_accessObj.m_pModel->vpT1Coefs[m_tIdx2[rgbIdx]][xoIdx * oneItplTSize];
					min1 = *(float*)p1;
					p1 += sizeof(float);
					range1 = *(float*)p1;
					p1 += sizeof(float);
					min2 = *(float*)p2;
					p2 += sizeof(float);
					range2 = *(float*)p2;
					p2 += sizeof(float);

					//cache
					Vec3* pTmp = &m_offset2DirCache[0];
					for(int oIdx = 0; oIdx < _T6k; ++oIdx)
					{
						integral += 
							m_curLightCoefs[rgbIdx][oIdx] * 
							(m_tW1[rgbIdx] * DeQuant8(*p1, min1, range1) + m_tW2[rgbIdx] * DeQuant8(*p2, min2, range2));
						++pTmp;
						++p1;
						++p2;
					}

#else				//the float version
					float* pT1 = &m_accessObj.m_pModel->vpT1Coefs[xoIdx * oneItplTSize * _TItplNum + m_tIdx1[rgbIdx] * oneItplTSize];
					float* pT2 = pT1 + oneItplTSize;
					Vec3* pTmp = &m_offset2DirCache[0];
					for(int oIdx = 0; oIdx < _T6k; ++oIdx)
					{
						integral += 
							m_curLightCoefs[rgbIdx][oIdx] * 
							(m_tW1[rgbIdx] * (*pT1) + m_tW2[rgbIdx] * (*pT2));
						++pT1;
						++pT2;
					}
#endif
					*(Transport.GetAddress(rgbIdx)) = *m_sigmaS.GetAddress(rgbIdx) * integral * m_SSWeight * m_SSCoef;
				}
			}
		
			if(_Fwo1)
			{
				*pColor = Transport * m_colorCoef / PI;
			}
			else
			{
				*pColor = Transport * m_colorCoef * FresnelT(Cos) / PI;
			}
			if((m_bDrawSS || m_bDrawMS) && _Spec && (Cos > Epsilon))
			{
				Vec3 reflectLocal = *pNorm * 2.f * Cos - viewLocal;
				Vec3 lightOrig = VecMultMat4Inv(&reflectLocal, &g_envTransform);
				//*pColor = *(m_cubeMap.Dir2NearestColor(-viewLocal));
				*pColor += *(m_cubeMap.Dir2NearestColor(lightOrig)) * FresnelR(Cos) * m_specCoef;
			}
		}
	}
}

//tone brightness
BOOL CPRTView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(m_bLoadedCubeMap)
	{
		if (m_bLBDown)	//light
		{
			if(zDelta > 0)//zoomIn
				m_SSCoef *= 1.1f;
			else if(zDelta < 0)
				m_SSCoef *= 0.9f;	
			printf("m_SSCoef=%f\n", m_SSCoef);
		}
		else if (m_bRBDown)	//specular
		{
			if(zDelta > 0)//zoomIn
				m_specCoef *= 1.05f;
				//m_specCoef *= 1.25f;
			else if(zDelta < 0)
				m_specCoef *= 0.95f;
				//m_specCoef *= 0.8f;	
			printf("m_specCoef=%f\n", m_specCoef);
		}
		else	//light+mesh, color
		{
			if(zDelta > 0)//zoomIn
				m_colorCoef *= 1.05f;// 1.25f;
			else if(zDelta < 0)
				m_colorCoef *= 0.95f; //0.8f;
			printf("m_colorCoef=%f\n", m_colorCoef);
		}
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CPRTView::DrawCube(float cubeSize)
{

	float fSize = cubeSize / 2.f;
	glBegin(GL_QUADS);

	//right face
	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fSize, -fSize, -fSize);			
	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fSize, -fSize, fSize);			
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fSize, fSize, fSize);			
	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fSize, fSize, -fSize);

	//left face 
	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fSize, -fSize, fSize);			
	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fSize, -fSize, -fSize);			
	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fSize, fSize, -fSize);			
	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fSize, fSize, fSize);

	//top face 
	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fSize, fSize, -fSize);			
	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fSize, fSize, -fSize);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fSize, fSize, fSize);			
	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fSize, fSize, fSize);			

	//bottom face 
	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fSize, -fSize, fSize);			
	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fSize, -fSize, fSize);			
	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fSize, -fSize, -fSize);			
	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fSize, -fSize, -fSize);

	//back face
	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fSize, fSize, fSize);			
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fSize, fSize, fSize);
	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fSize, -fSize, fSize);			
	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fSize, -fSize, fSize);			

	//front face
	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fSize, -fSize, -fSize);			
	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fSize, -fSize, -fSize);			
	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fSize, fSize, -fSize);			
	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fSize, fSize, -fSize);			

	glEnd();
}

void CPRTView::OnInitialUpdate()
{
	GetParent()->ShowWindow(SW_SHOWMAXIMIZED);
	CView::OnInitialUpdate();
}

void CPRTView::InitCubeMap()
{
	glDeleteTextures(1, &m_texCube);
	glGenTextures(1, &m_texCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, m_texCube);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);		
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);		
	for (int side = 0; side < 6; ++side) 
	{
		//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + side, 0, GL_RGB, m_cubeRes, m_cubeRes, 0, GL_RGB, GL_FLOAT, m_cubeMap.m_pDownCube + side * m_cubeRes * m_cubeRes);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + side, 0, GL_RGB, m_cubeMap.m_nFaceWidth, m_cubeMap.m_nFaceHeight, 0, GL_RGB, GL_FLOAT, m_cubeMap.m_pCube + side * m_cubeMap.m_nFaceWidth * m_cubeMap.m_nFaceHeight);
	}
	CheckErrorsGL("Init Cubemap");
}
			
void CPRTView::OnOpenCubeMap()
{
	char szFilter[] = "pfm Files (*.pfm)|*.pfm|All Files (*.*)|*.*||";

	CFileDialog openDlg(TRUE,NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter,
		this);
	CString szFilepath;
	char workdir[128];
	_getcwd( workdir, 128);

	if(openDlg.DoModal() == IDOK)
	{
		szFilepath = openDlg.GetPathName();

		if(m_cubeMap.LoadPFM((char*)(LPCSTR)szFilepath))
		{
			InitCubeMap();
			m_bLoadedCubeMap = true;
			
			if(m_bLoadedTdCoefs || m_bLoadedECoefs)
				m_bDrawMS = true;
			if(m_bLoadedT1Coefs)
				m_bDrawSS = true;

			CalcLightCoef();
			CalcColor();
			ChangeDrawElements();
		}
		////for test passed	woItpl binlinear
		//int ofWi[4];
		//float wWi[4];
		//for(int side = 0; side < 6; ++side)
		//{
		//	for(int offset = 0; offset < m_cubeRes * m_cubeRes; ++offset)
		//	{
		//		Vec3 dir = m_cubeMap.Offset2Dir(side, offset, m_cubeRes);
		//		m_cubeMap.Dir2CubeOffsetBilinear(dir, m_cubeRes, ofWi, wWi);
		//		for(int it = 0; it < 4; ++it)
		//			assert(abs(side * m_cubeRes * m_cubeRes + offset - ofWi[it]) <= 33);
		//		//int j = m_cubeMap.Dir2Offset(dir, m_cubeRes, side);
		//		//assert (offset == j);
		//	}
		//}
	}
	_chdir(workdir);
}

void CPRTView::OnUpdateEditDrawNormal(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawNormal);
}

void CPRTView::OnUpdateEditDrawNormalMap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawNormalMap);
}

void CPRTView::OnUpdateEditDrawVisRatio(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawVisRatio);
}

void CPRTView::OnEditDrawLight()
{
	m_bDrawLight = !m_bDrawLight;

	if(m_bDrawLight)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
	CalcLightCoef();

	Invalidate();
}

void CPRTView::OnUpdateEditDrawLight(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawLight);
}

void CPRTView::OnDiffuseE()
{
	_DiffuseE = !_DiffuseE;
	CalcLightCoef();
	CalcColor();
	Invalidate();
}

void CPRTView::OnFwi1()
{
	_Fwi1 = !_Fwi1;
	CalcLightCoef();
	CalcColor();
	Invalidate();
}

void CPRTView::OnFwo1()
{
	_Fwo1 = !_Fwo1;
	CalcLightCoef();
	CalcColor();
	Invalidate();
}

void CPRTView::OnUpdateDiffuseE(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(_DiffuseE);
}

void CPRTView::OnUpdateFwi1(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(_Fwi1);
}

void CPRTView::OnUpdateFwo1(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(_Fwo1);
}

void CPRTView::OnUsecoefnum4()
{
	m_useCoefNum = 4;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum8()
{
	m_useCoefNum = 8;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum16()
{
	m_useCoefNum = 16;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUseCoefNum32()
{
	m_useCoefNum = 32;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum64()
{
	m_useCoefNum = 64;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum96()
{
	m_useCoefNum = 96;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum128()
{
	m_useCoefNum = 128;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUsecoefnum256()
{	
	m_useCoefNum = 256;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUpdateUsecoefnum4(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 4);
}

void CPRTView::OnUpdateUsecoefnum8(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 8);
}

void CPRTView::OnUpdateUsecoefnum16(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 16);

}

void CPRTView::OnUpdateUsecoefnum32(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 32);
}

void CPRTView::OnUpdateUsecoefnum64(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 64);
}

void CPRTView::OnUpdateUsecoefnum96(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 96);

}

void CPRTView::OnUpdateUsecoefnum128(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 128);

}

void CPRTView::OnUpdateUsecoefnum256(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_useCoefNum == 256);

}

////////////////////////////////////////////////////////////////////////// SSIso

//return sampleNum=16 intrude distances from verts, whose sampling path depth is dm. intrudes should have sampleNum elements!!!
void CPRTView::SSSampleIntrude(float* intrudes, float* pdfs, int sampleNum, float dm)
{
	//assert(intrudes && (sampleNum > 0) && (dm > 0));
	
	//if(dm < Epsilon)	//ykdeb
	//{
	//	for(int i = 0; i < sampleNum; ++i)
	//	{
	//		intrudes[i] = pdfs[i] = 0;
	//	}
	//	return;
	//}
	if(_MC)
	{
		float lamda = 1.f - expf(- m_sigmaTLumi * dm); //if dm->0, then lamda->0
		for(int i = 0; i < sampleNum; ++i)
		{
			float ksi = (i + 0.5f) / (float)sampleNum;		//NOTE "+" since i starts from 0!
			intrudes[i] = - logf(1 - lamda * ksi) / m_sigmaTLumi;	//if dm->0, then->0
			pdfs[i] = m_sigmaTLumi * expf(-m_sigmaTLumi * intrudes[i]) / lamda; //if dm->0 ,then->inf
		}
	}
	else
	{
		for(int i = 0; i < sampleNum; ++i)
		{
			intrudes[i] = dm * (i + 0.5f) / (float)sampleNum;
			pdfs[i] = 1.f / dm;	//DEBUGGED!!!
		}
	}
	////for test: assert sum(pdf*s) = 1, not passed
	//float* pdf = new float[sampleNum];	//gama
	//for(int i = 0; i < sampleNum; ++i)
	//{
	//	pdf[i] = sigmaTLumi * expf(- sigmaTLumi * intrudes[i]) / lamda;
	//}

	//float sum = 0.f;
	//for(int i = 0; i < sampleNum; ++i)
	//{
	//	sum += pdf[i] * intrudes[i];
	//}
	//assert(fabs(sum - 1.f) < 1.0e-5);
}

void CPRTView::SubsurfLookatVBOs()
{
	// Get Pointers To The GL Functions
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");

	Vec3* pColors = new Vec3[m_accessObj.m_pModel->nVertices + 1];
	for(unsigned int i = 0; i <= m_accessObj.m_pModel->nVertices; ++i)
	{
		pColors[i] = (m_accessObj.m_pModel->vpVertexNormals[i] + 1) * 0.5f; //-1,1 -> 0,1
	}

	glEnableClientState( GL_VERTEX_ARRAY );						// Enable Vertex Arrays
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// Generate And Bind The Vertex Buffer
	glGenBuffersARB( 1, &m_nVBOVertices );					// Get A Valid Name
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			// Bind The Buffer
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), m_accessObj.m_pModel->vpVertices, GL_STATIC_DRAW_ARB );
	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		// Set The Vertex Pointer To The Vertex Buffer

	glGenBuffersARB( 1, &m_nVBOColors );	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOColors );			// Bind The Buffer
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,  sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), pColors, GL_STATIC_DRAW_ARB);
	glColorPointer(3, GL_FLOAT, 0, (char *) NULL);
	delete[] pColors;	
	pColors = NULL;
}

void DrawTexRect()
{
	glBegin(GL_QUADS);
	 glTexCoord2f(0.f, 0.f);
	 glVertex2f(-1, -1);
	 glTexCoord2f(1.f, 0.f);
	 glVertex2f( 1, -1);
	 glTexCoord2f(1.f, 1.f);
	 glVertex2f(1, 1);
	 glTexCoord2f(0.f, 1.f);
	 glVertex2f(-1, 1);
	glEnd();
}

inline bool mod_lesser (const float& elem1, const float& elem2 )
{
	return elem1 < elem2;
};

/*inline*/ class Quantize8addHalf //[min,max]->[0.5f,255.5f]. add 0.5 for later static_cast<byte> use.
{
private:
	float myMin, myRange;
public:
	Quantize8addHalf(const float& _min, const float& _range) : myMin( _min ), myRange(_range){}
	void operator()(float& elem)const {elem = (elem - myMin) / myRange * 255.f + 0.5f;}
};
class Set2Zero
{
public:
	Set2Zero(){}
	void operator()(float& elem)const {elem = 0;}
};

//for each vert
//	for each sigma
//		calc T6k wi's T1
//		Coef(T6k)
//		repack into slijm order
//		keepLoss(64)
//		Quant8Write
//render to depth,stencil,norm to get si,vis,ni, accumulate them in texture by FBO and shader, add_blend in texblend
void CPRTView::OnComputeSaveT1()
{
	//for each vert
	//	for each side
	//		for each xp
	//			pass1: enable(depth,sten), fbo.bind(normTex, zTex). draw. vs: norm=MVP(glNorm). z=(MV*V).z, ps:depth=z,color=norm
	//			readPixel(sten), texSubImg(stenTex)
	//			pass2: glBlend(add)(if xp1, clear color)), disable(depth,sten), fbo.unbind(normTex, zTex), fbo.bind(t1Tex), drawviewport. vs:(no), ps:fragColor = sample(normTex,zTex,stenTex), disable(blend)
	//		getTexImg(texT1)
	//	haar T1OneVert

	if((!m_bLoadedObj) || (!m_bLoadedDm))
	{
		printf("!m_bLoadedObj || (!m_bLoadedDm)\n  ");
		return;
	}
	char tStr[3];
	vector<FILE*> pTFiles(_TItplNum);
	for(int tIdx = 0; tIdx < _TItplNum; ++tIdx)
	{
		itoa(tIdx, tStr, 10);
		CString fnT1C = m_filenameNoSuffix + ".Qt" + tStr;		
		if((pTFiles[tIdx] = fopen(((char*)(LPCSTR)fnT1C), "wb")) == NULL)
		{
			printf("failed open writing T1 coefs. \n ");
			return;
		}
	}
	printf("start compute and save T1 Coefs...\n ");
	DWORD dwTmTtlSrt;
	DWORD dwTmTtlEnd;
	dwTmTtlSrt = GetTickCount();
	GLint _currentDrawbuf = 0;
	glGetIntegerv(GL_DRAW_BUFFER, &_currentDrawbuf); // Save the current Draw buffer

	vector<float>intrudes(m_SSSampleNum, 0.f);
	vector<float>pdfs(m_SSSampleNum, 0.f);
	vector<float>pKT1(_T6k * _TItplNum, 0.f);
	vector<float>pOneT1Loss(m_T1CoefNum, 0.f);
	vector<short int> pOneT1IdxLoss(m_T1CoefNum, 0);
	vector<float>SqCoef(_T6k, 0.f);

	//SubsurfLookat's InitGL
	glViewport(0, 0, m_cubeRes, m_cubeRes);
	glClearColor(0,0,0,0);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);//this is right.
	glDisable(GL_CULL_FACE);	//important!!
	glClearDepth(1.0);
	glClearStencil(0x0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1, 0.001f, m_objBboxSize * 2);	//near:intrude[0] is minimal distance that should be covered in [near,far]. 
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//VBO: can't delete these 4 lines, or there'll be access violation!
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
	glEnableClientState( GL_VERTEX_ARRAY );	
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glGenBuffersARB( 1, &m_nVBOVertices );	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), &m_accessObj.m_pModel->vpVertices[0], GL_STATIC_DRAW_ARB );
	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );
	glGenBuffersARB( 1, &m_nVBOColors );	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOColors );
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,  sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1),  &m_accessObj.m_pModel->vpVertexNormals[0], GL_STATIC_DRAW_ARB);
	glNormalPointer(GL_FLOAT, 0, (char *) NULL);
	CheckErrorsGL("VBO");

	//FBO
	glewInit();
	GLuint NiSiTex, foozTex;
	FramebufferObject fb;

	glGenTextures(1, &NiSiTex);	
	glBindTexture(GL_TEXTURE_2D, NiSiTex);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA16F_ARB, m_cubeRes, m_cubeRes, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	fb.AttachTexture(GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, NiSiTex, 0, 0);

	//but the foolish "invalid_operation" pops up at readpixels(stencil/depth), when stencil or depth is not attached ever. so here attach a useless depth, only stencil is useful
	//!!!now this is again impossible.. !!NOW can be used by shader!! read through .g: oss.sgi\packed_depth_stencil(1).txt: "blue    green   red     alpha"
	//before that, exists only for later "glReadPixels(...Stencil...), which won't use it
	glGenTextures(1, &foozTex);
	glBindTexture(GL_TEXTURE_2D, foozTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT, m_cubeRes, m_cubeRes, 0,GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	fb.AttachTexture(GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, foozTex);//f**k!oss.sgi\packed_depth_stencil(1).txt: "If both attachment points FRAMEBUFFER_DEPTH_ATTACHMENT.."
	fb.AttachTexture(GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, foozTex);

	GLenum* pDrawbuffers = new GLenum[_TItplNum / 4];//my card only support max 4 drawbuffers.
	GLuint* pT1Tex = new GLuint[_TItplNum / 4]; //4
	glGenTextures(_TItplNum / 4, pT1Tex); 
	for(int i = 0; i < _TItplNum / 4; ++i)
	{
		pDrawbuffers[i] = GL_COLOR_ATTACHMENT0_EXT + i;
		glBindTexture(GL_TEXTURE_2D, pT1Tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA16F_ARB, m_cubeRes, m_cubeRes, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	//Shader
	aShaderObject* Shader1 = 0;  
	aVertexShader* VertexShader1 = 0;
	aFragmentShader* FragmentShader1 = 0;
	aShaderObject* myShader1 = DefaultShaderSetup(VERT_FILE1, FRAG_FILE1, Shader1, VertexShader1, FragmentShader1);	
	aShaderObject* Shader2 = 0;  
	aVertexShader* VertexShader2 = 0;
	aFragmentShader* FragmentShader2 = 0;
	aShaderObject* myShader2 = DefaultShaderSetup(VERT_FILE2, FRAG_FILE2, Shader2, VertexShader2, FragmentShader2);	
	if((myShader1 == NULL) || (myShader2 == NULL))
	{
		cout<<"failed DefaultShaderSetup!\n";
		DefaultCleanup(Shader1, VertexShader1, FragmentShader1);
		DefaultCleanup(Shader2, VertexShader2, FragmentShader2);
		if(m_nVBOVertices || m_nVBOColors)
		{
			unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOColors };
			glDeleteBuffersARB( 2, nBuffers );						// Free The Memory
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glDisable(GL_STENCIL_TEST);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		InitGL();
		return;
	}
	CheckErrorsGL("Shader");
	fb.Bind();
	//start render
	m_bPreRendering = true;
	Vec3 center = Vec3(0,0,0);
	Vec3 up = Vec3(0,0,0);
	Vec3* pVert = &m_accessObj.m_pModel->vpVertices[0];
	Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[0];
	Vec3* pDm =  &m_accessObj.m_pModel->vpDm[0];
	Vec3 VertXp = Vec3(0, 0, 0);
	//opt
	int fboId = fb.m_fboId;
	GLhandleARB ShaderObject1 = myShader1->ShaderObject;
	GLhandleARB ShaderObject2 = myShader2->ShaderObject;
	//Quant8
	float vMin, vRange;
	vector<float>::iterator iter;

//FILE* fdeb = fopen("deb.txt", "wt");
	for (unsigned int vIdx = 0; vIdx < m_accessObj.m_pModel->nVertices; ++vIdx)
	{ 
		++pVert; //obj index starts from 1
		++pNorm;
		++pDm;
		
		SSSampleIntrude(&intrudes[0], &pdfs[0], m_SSSampleNum, pDm->x);
		for(int side = 0; side < 6; side++)
		{
			for(int p = 0; p < m_SSSampleNum; ++p)
			{
				glStencilFunc(GL_ALWAYS, 0x1, 0x1);
				glStencilOp (GL_INCR, GL_INCR, GL_INCR);
				//pass1
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, NiSiTex, 0);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
				//draw
				glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);//this color is attach0, norm!!
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				VertXp = *pVert - *pNorm * intrudes[p]; //intrude to xp. since early here we need know intrudes, which is dependent on sigmaT, we can't use MC importance integration now...
				m_cubeMap.EyeSide2CenterUp(&VertXp, side, &center, &up);
				gluLookAt(VertXp.x, VertXp.y, VertXp.z, center.x, center.y, center.z, up.x, up.y, up.z);
				glUseProgramObjectARB(ShaderObject1);
				glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);
				glFlush();
				glUseProgramObjectARB(0);
				//now: NiSiTex.rgb is Ni, .a is Si; stencil has Vis. test passed
				//ykdeb glStencilFunc(GL_LEQUAL, 0x1, 0xFFFFFFFF); //<=1, so that very thin can also calc
				glStencilFunc(GL_EQUAL, 0x1, 0xFFFFFFFF);
				glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
				glEnable (GL_BLEND);
				glBlendFunc (GL_ONE, GL_ONE);
				//pass2
				for(int i = 0; i < _TItplNum / 4; ++i)
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D,  pT1Tex[i], 0);
				}
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
					glDrawBuffers(_TItplNum / 4, pDrawbuffers);//pT1Tex
				//draw
				if(p == 0)
					glClear(GL_COLOR_BUFFER_BIT);//this color is attach1, t1Tex!

				glUseProgramObjectARB(ShaderObject2);
				GLint loc = glGetUniformLocationARB(ShaderObject2, "intrudepdf");
				glUniform2fARB(loc, intrudes[p], pdfs[p]);
				loc = glGetUniformLocationARB(ShaderObject2, "side");
				glUniform1iARB(loc, side);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, NiSiTex);
				loc = glGetUniformLocationARB(ShaderObject2, "NiSiSampler");
				glUniform1iARB(loc, 0);
				//DrawTexRect();				
				glBegin(GL_QUADS);
				glTexCoord2f(0.f, 0.f);
				glVertex2f(-1, -1);
				glTexCoord2f(1.f, 0.f);
				glVertex2f( 1, -1);
				glTexCoord2f(1.f, 1.f);
				glVertex2f(1, 1);
				glTexCoord2f(0.f, 1.f);
				glVertex2f(-1, 1);
				glEnd();
				glFlush();
				glUseProgramObjectARB(0);
				CheckErrorsGL("finish pass2");
				//for(int i = 0; i < 4; ++i)
				//{
				//	glBindTexture(GL_TEXTURE_2D, pT1Tex[i]);
				//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &pKT1[i * _T6k + side * _T1k]);
				//	CString sss = "k";
				//	char strSide[2], strK[2];
				//	_itoa(side, strSide, 10);
				//	_itoa(i, strK, 10);
				//	sss = sss + strK + "side" + strSide + ".pfm";
				//	m_cubeMap.WritePFMRaw(sss, &pKT1[i * _T6k + side * _T1k], m_cubeRes, m_cubeRes);
				//}
				glDisable(GL_BLEND);
			}
			//now there're k = _TItplNum pT1Tex results for this side. copy all the _TItplNum sides into corresponding places in pKT1
			float* pT1idx = &pKT1[side * _T1k];
			for(int k = 0; k < _TItplNum / 4; ++k)
			{
				glBindTexture(GL_TEXTURE_2D, pT1Tex[k]); //sigma0,...sigma7, 8 texRects in pT1Tex
				//yksse: should test the order: red is sigma1 or sigma4?
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pT1idx);
				pT1idx += _T6k;
				glGetTexImage(GL_TEXTURE_2D, 0, GL_GREEN, GL_FLOAT, pT1idx);
				pT1idx += _T6k;
				glGetTexImage(GL_TEXTURE_2D, 0, GL_BLUE, GL_FLOAT, pT1idx);
				pT1idx += _T6k;
				glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_FLOAT, pT1idx);
				pT1idx += _T6k;
			}
		}
#if 1
		// byte dequant version
		for(int i = 0; i < _TItplNum; ++i)
		{
			iter = min_element(pKT1.begin() + i*_T6k, pKT1.begin() + _T6k + i*_T6k, mod_lesser);
			vMin = *iter;
			iter = max_element(pKT1.begin() + i*_T6k, pKT1.begin() + _T6k + i*_T6k, mod_lesser);
			vRange =  *iter - vMin;
			size_t n = fwrite(&vMin, sizeof(float), 1, pTFiles[i]);
			assert(n == 1);
			n = fwrite(&vRange, sizeof(float), 1, pTFiles[i]);
			assert(n == 1);

			if(vRange > Epsilon)
				for_each(pKT1.begin() + i*_T6k, pKT1.begin() + _T6k + i*_T6k, Quantize8addHalf(vMin, vRange));//[0.5f, 255.5f]
			else
				for_each(pKT1.begin() + i*_T6k, pKT1.begin() + _T6k + i*_T6k, Set2Zero());
			byte tmpb;
			for(iter = pKT1.begin() + i*_T6k; iter < pKT1.begin() + _T6k + i*_T6k; ++iter)
			{
				tmpb = static_cast<byte>(*iter);
				n = fwrite(&tmpb, sizeof(byte), 1, pTFiles[i]);
				assert(n == size_t(1));
			}
		}
#else
		//unquantized float version
		size_t n = fwrite(&pKT1[0], sizeof(float), _TItplNum * _T6k, pTFiles[i]);
		assert(n = _TItplNum * _T6k);
#endif
		if(vIdx % 100 == 0)
		{
			dwTmTtlEnd = GetTickCount();
			std::cout<<"T1k coefs of vert "<<vIdx<<" of "<< m_accessObj.m_pModel->nVertices<<" vertices finished, time: "<< (dwTmTtlEnd - dwTmTtlSrt) / 1000<<" \n";
		}
	}
//fclose(fdeb);
	std::cout<<"Finish T1k coefs ! \n";

	for(int k = 0; k < _TItplNum; ++k)
	{
		fclose(pTFiles[k]);
	}
	printf("Finish T1k coefs ! \n");

	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	fb.Disable();
	if(pDrawbuffers)
	{
		delete[] pDrawbuffers;
		pDrawbuffers = NULL;
	}
	if(pT1Tex)
	{
		delete[] pT1Tex;
		pT1Tex = NULL;
	}
	DefaultCleanup(Shader1, VertexShader1, FragmentShader1);
	DefaultCleanup(Shader2, VertexShader2, FragmentShader2);
	if(m_nVBOVertices || m_nVBOColors)
	{
		unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOColors };
		glDeleteBuffersARB( 2, nBuffers );
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	m_bPreRendering = false;

	////load
	//CString T1CName = m_filenameNoSuffix + ".T1C";
	//if(m_accessObj.LoadT1Coefs((char*)(LPCSTR)T1CName))	
	//{
	//	m_bLoadedT1Coefs = true;
	//}
	//if(m_bLoadedT1Coefs && m_bLoadedCubeMap)
	//	m_bDrawSS = true;
		InitGL();
	if(m_bLoadedT1Coefs && m_bLoadedCubeMap)
		m_bDrawSS = true;
	InitGL();
}

void CPRTView::SubsurfLookatInitGL()
{
	//reset GL for subsurfVisib use	
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);

	glViewport(0, 0, m_cubeRes, m_cubeRes);
	glClearColor(0,0,0,0);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0x0);
	
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp (GL_INCR, GL_INCR, GL_INCR);

	//Reset The Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1, 0.001f, m_objBboxSize * 2);	//near:intrude[0] is minimal distance that should be covered in [near,far]. 
	//for m_bbox=10, dm=0.1 is suitable. For dm=0.1 case,intrude[0]=0.003. 
	//so for near, 0.001 is enough.far:m_objBboxSize * sqrtf(3)) is enough
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//VBO
	// Get Pointers To The GL Functions
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");

	Vec3* pColors = new Vec3[m_accessObj.m_pModel->nVertices + 1];
	for(unsigned int i = 0; i <= m_accessObj.m_pModel->nVertices; ++i)
	{
		pColors[i] = (m_accessObj.m_pModel->vpVertexNormals[i] + 1) * 0.5f; //-1,1 -> 0,1
	}

	glEnableClientState( GL_VERTEX_ARRAY );						// Enable Vertex Arrays
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// Generate And Bind The Vertex Buffer
	glGenBuffersARB( 1, &m_nVBOVertices );					// Get A Valid Name
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			// Bind The Buffer
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), m_accessObj.m_pModel->vpVertices, GL_STATIC_DRAW_ARB );
	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		// Set The Vertex Pointer To The Vertex Buffer

	glGenBuffersARB( 1, &m_nVBOColors );	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOColors );			// Bind The Buffer
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,  sizeof(Vec3) * (m_accessObj.m_pModel->nVertices + 1), pColors, GL_STATIC_DRAW_ARB);
	glColorPointer(3, GL_FLOAT, 0, (char *) NULL);
	delete[] pColors;	
	pColors = NULL;
}

//eye at pXp, lookat _T6k dirs, to get vis,si,ni, to accum to T1
void CPRTView::SubsurfLookat(Vec3* pXp, byte* pSSVisOneVert, float* pSiOneVert, Vec3* pNiOneVert)
{
	//si'
	GLdouble siX, siY, siZ;
	GLdouble IdModel[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	GLdouble Proj[16];
	glGetDoublev(GL_PROJECTION_MATRIX, Proj);
	GLint viewport[16];
	glGetIntegerv(GL_VIEWPORT, viewport);

	Vec3 center = Vec3(0,0,0);
	Vec3 up = Vec3(0,0,0);
	for(int side = 0; side < 6; side++)
	{
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		m_cubeMap.EyeSide2CenterUp(pXp, side, &center, &up);
		gluLookAt(pXp->x, pXp->y, pXp->z,
			center.x, center.y, center.z,
			up.x, up.y, up.z);
		glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);
		glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &pSSVisOneVert[side * _T1k]); //subsurf visibs
		glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_DEPTH_COMPONENT, GL_FLOAT, &pSiOneVert[side * _T1k]);
		glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_RGB, GL_FLOAT, &pNiOneVert[side * _T1k]);
	}

	//Ni
	Vec3* pNi = &pNiOneVert[0];
	//Sp
	float* pSi = &pSiOneVert[0];
	for(int i = 0; i < _T6k; ++i)
	{
		//Ni
		*pNi = (*pNi * 2.f) - 1.f;
		pNi->Normalize();
		++pNi;
		//Sp
		gluUnProject(GLdouble(i % m_cubeRes), GLdouble(i / m_cubeRes),  GLdouble(*pSi), IdModel, Proj, (GLint*)viewport, &siX, &siY, &siZ);
		*pSi = (float)-siZ;
		++pSi;
	}
	//side0 fliplr
	Fliplr(&pSiOneVert[0], m_cubeRes, m_cubeRes);
	Fliplr(&pNiOneVert[0], m_cubeRes, m_cubeRes);
	Fliplr(&pSSVisOneVert[0], m_cubeRes, m_cubeRes);
}

void	CPRTView::SubsurfBlend(Vec3* pOneT1, byte* pSSVisOneVert, float* pSiOneVert, Vec3* pNiOneVert, float* pIntrude, float* pPdf)
{
	float Cos;
	int curOffset = 0;
	//Vec3 Wi;
	float Si1;
	Vec3* pT1 = pOneT1;
	byte* pVis = pSSVisOneVert;
	float* pSi = pSiOneVert;
	Vec3* pNi = pNiOneVert;
	for(int side = 0; side < 6; ++side)
		for(int offset = 0; offset < _T1k; ++offset)
		{
			//Wi = m_cubeMap.Offset2Dir(side, offset, m_cubeRes);
			//curOffset = side * _T1k + offset;
			if(*pVis == 1) //stencil==1: visible.
			{
				Cos = Dot(*pNi, m_cubeMap.Offset2Dir(side, offset, m_cubeRes));
				//Cos = Dot(*pNi, Wi);
				if(Cos > Epsilon)
				{
					Si1 = pSiOneVert[curOffset] * (Cos / sqrtf(1 - (1 - Cos * Cos) / m_yita / m_yita));
					if(_Fwi1)
						*pT1 += EXPF(- m_sigmaT * (*pIntrude + Si1)) / *pPdf;
					else
						*pT1 += EXPF(- m_sigmaT * (*pIntrude + Si1)) / *pPdf * FresnelT(Cos);
				}
			}
			else if(*pVis == 0)//this vertex is invalid, all its samples should be discarded, shouldn't let pT1 to continue to accumulate
			{
				//cout<<"ft!\n";
				break;
			}
			++curOffset;
			++pT1;
			++pVis;
			++pSi;
			++pNi;
		}
}


//input: Xp, side. output: stencil->pSSVis; depth->Sp; color->Ni(Norm xi).
//standing at pXp, which is below surface, lookAt 1 of 6 sides of resolution m_pModel's visibRes, 
//fill the 2nd visib map of pVert into subsurfvisibs. fill normals of intersected (interpolated) pixel-normals into Norm, fill depths into spPrime.
int	CPRTView::SubSurfVisibleLightSide(byte* pSSVisOneSide, float* pSpOneSide, Vec3* pNiOneSide, Vec3* pXp, int side, long int& debugXpOutSum)
{
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Vec3 center = Vec3(0,0,0);
	Vec3 up = Vec3(0,0,0);
	m_cubeMap.EyeSide2CenterUp(pXp, side, &center, &up);
	gluLookAt(pXp->x, pXp->y, pXp->z,
		center.x, center.y, center.z,
		up.x, up.y, up.z);

	glDrawElements(GL_TRIANGLES, 3 * m_accessObj.m_pModel->nTriangles, GL_UNSIGNED_INT, &m_accessObj.m_pModel->TrianglesIndices[0]);
	//glCallList(m_displayListId);

	glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pSSVisOneSide); //subsurf visibs
	glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_DEPTH_COMPONENT, GL_FLOAT, pSpOneSide);
	glReadPixels(0, 0, m_cubeRes, m_cubeRes, GL_RGB, GL_FLOAT, pNiOneSide);

	//count visRatio, normalize pNi
	int visCount = 0;
	GLubyte* pStencil = pSSVisOneSide;
	Vec3* pNi = pNiOneSide;
	for(int i = 0; i < m_cubeRes * m_cubeRes; ++i)
	{	
		*pNi = (*pNi * 2.f) - 1.f;
		pNi->Normalize();	//norm
		if(*pStencil == 1)//subsurfvisible
		{
			visCount++;
		}		
		else if(*pStencil == 0) //shouldn't happen!
		{
			debugXpOutSum++;
			//printf("sample == 0\n");
			//*pStencil = 1;	//modify it to invisible
		}
		//else //for test passed
		//	assert((*pStencil > 1) && (*pStencil <= 255));	
		++pStencil;
		++pNi;
	}
	return visCount;	
}

void CPRTView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!m_bLoadedObj)
		return;

	if(m_bDrawSubsurfVisratio)
	{
		m_svrKeyDownIdx = (m_svrKeyDownIdx + 1) % m_SSSampleNum;
		printf("%dth subvisratio\n", m_svrKeyDownIdx);
	}
	
	if((m_bLoadedObj) && (nChar == 0x57))//VK_W))
	{
		m_fillMode = (m_fillMode + 1) % 2;	

		if(m_fillMode == 0)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if(m_fillMode == 1)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		printf("fill mode changed\n");
	}

	if((m_bLoadedTdCoefs) && (nChar == 0x4d))//VK_M))
	{
		m_bDrawMS = !m_bDrawMS;
		printf("m_bDrawMS = %d\n", m_bDrawMS);
	}
	if((m_bLoadedTdCoefs) && (nChar == VK_UP))
	{
		m_MSWeight *= 1.1f;
		printf("m_MSWeight = %f\n", m_MSWeight);
	}
	if((m_bLoadedTdCoefs) && (nChar == VK_DOWN))
	{
		m_MSWeight *= 0.9f;
		printf("m_MSWeight = %f\n", m_MSWeight);
	}

	if(m_bLoadedT1Coefs && (nChar == 0x53))//VK_S))
	{
		m_bDrawSS = !m_bDrawSS;
		printf("m_bDrawSS = %d\n", m_bDrawSS);
	}
	if(m_pECoefsTree && (nChar == 0x42) && GetKeyState(VK_CONTROL))//VK_B))
	{
		m_bDrawBBox = !m_bDrawBBox;
	}
	if(_Tex && (!m_textureImage.empty()) && (nChar == 0x54)) //VK_T
	{
		if(GetKeyState(VK_CONTROL) < 0) //ctrl + t: decrease
		{
			m_texAlpha = unsigned char(max(10, min(255, m_texAlpha * 0.9f)));
		}
		else
		{
			m_texAlpha = unsigned char(max(10, min(255, m_texAlpha * 1.1f)));
		}
		printf("m_texAlpha = %d\n",m_texAlpha);
		for(int i = 0; i < m_texW * m_texH; ++i)
			m_textureImage[i * 4 + 3] = m_texAlpha;
	}
	//record screen in fixed directions : in numpad, 1,2,3 means rotate about x, y, z axis
	//ref to CADtextbook of Pan Yunhe P.64, 
	//mat of rot about x-axis:[1 0 0; 0 cos sin; 0 -sin cos], y-axis:[cos 0 -sin; 0 1 0; sin 0 cos], z-axis:[cos sin 0; -sin cos 0; 0 0 1]
	if((nChar == VK_NUMPAD1) || (nChar == VK_NUMPAD2) || (nChar == VK_NUMPAD3))
	{
		if(nChar == VK_NUMPAD1)//rot x 90
		{
			Matrix3fT tmp = {1.f, 0.f, 0.f,		0.f, 0.f, 1.f,		0.f, -1.f, 0.f};
			Mat3Assign(&g_eyeThisRot, &tmp);
		}
		else if(nChar == VK_NUMPAD2)//rot y 90
		{
			Matrix3fT tmp = {0.f, 0.f, -1.f,	0.f, 1.f, 0.f,		1.f, 0.f, 0.f};
			Mat3Assign(&g_eyeThisRot, &tmp);
		}
		else if(nChar == VK_NUMPAD3)//rot z 90
		{
			Matrix3fT tmp = {0.f, 1.f, 0.f,		-1.f, 0.f, 0.f,		0.f, 0.f, 1.f};
			Mat3Assign(&g_eyeThisRot, &tmp);
		}
		Matrix3fMulMatrix3f(&g_eyeThisRot, &g_eyeLastRot);
		Matrix4fSetRotationFromMatrix3f(&g_modelTransform, &g_eyeThisRot);	// Set Our Final Transform's Rotation From This One
		g_eyeLastRot = g_eyeThisRot;
	}

	CalcLightCoef();
	CalcColor();

	ChangeDrawElements();
	return CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPRTView::OnDrawSubsurfVisRatio()
{	
	if(m_bLoadedSubsurfVisRatio)
	{
		m_bDrawSubsurfVisratio = !m_bDrawSubsurfVisratio;
		Invalidate();
	}
	else
	{
		printf("!m_bDrawSubsurfVisratio\n");
	}
}

void CPRTView::OnUpdateDrawSubsurfVisRatio(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawSubsurfVisratio);
}


//depth : using a ray tracer by shooting a ray along the negative normal direction and detecting the intersection. used by SSSample integration upper limit
void CPRTView::OnComputeSaveDm()
{
	if((!m_bLoadedObj))
	{
		printf("!m_bLoadedObj\n ");
		return;
	}
	CString fnDm = m_filenameNoSuffix + ".Dm";
	FILE* f;
	if((f = fopen(((char*)(LPCSTR)fnDm), "wb")) == NULL)
	{
		printf("failed open fnDm \n ");
		return;
	}
	printf("start compute save Dm...\n ");

	assert(m_accessObj.m_pModel);
	m_accessObj.m_pModel->vpDm.clear();
	m_accessObj.m_pModel->vpDm.resize(m_accessObj.m_pModel->nVertices + 1);

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, 1, 1);  //(0,0,1,1)won't work, will die ugly
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//zNear, zFar 	The distances to the nearer and farther depth clipping planes. These distances are negative if the plane is to be behind the viewer. 
	glOrtho(-1, 1, -1, 1, 0, m_objBboxSize * 2.f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	Vec3* pV = &m_accessObj.m_pModel->vpVertices[1]; //starts from 1;
	Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[1];
	Vec3* pDm = &m_accessObj.m_pModel->vpDm[1];
	float depth = 0;
	Vec3* pVert = NULL;
	Vec3 vecX;
	GLubyte color;
	int nBadNorm = 0, n01 =0, n001 =0, n0001 = 0;

	Vec3 vecRand = Vec3(0.2f, 0.3f, 0.5f);
	vecRand.Normalize();
	for(unsigned int vIdx = 0; vIdx < m_accessObj.m_pModel->nVertices; ++vIdx)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		vecX = Cross(*pNorm, vecRand);//center is in the viewport center, so up is random. but Up should not parellel norm
		gluLookAt(pV->x - Epsilon * pNorm->x, pV->y - Epsilon * pNorm->y, pV->z - Epsilon * pNorm->z, 
			pV->x - pNorm->x, pV->y - pNorm->y, pV->z - pNorm->z, vecX.x, vecX.y, vecX.z); //so that Center - Eye = -norm direction

		glBegin(GL_TRIANGLES);
		for(unsigned int i = 0; i < m_accessObj.m_pModel->nTriangles; i++)
		{
			for(int j = 0; j < 3;j++)
			{
				pVert = &m_accessObj.m_pModel->vpVertices[(m_accessObj.m_pModel->pTriangles[i]).vindices[j]];
				glVertex3f(pVert->x, pVert->y, pVert->z);
			}
		}
		glEnd();
		glFlush();

		glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth); 
		glReadPixels(0, 0, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &color); 

		pDm->x = pDm->y = pDm->z = depth * m_objBboxSize * 2.f;	//maxCoord * 2 is the Ortho size. queried depth of [0,1] should be scaled back by the ortho size
		if(pDm->x < 0.01)
		{
			++n01;
			if(pDm->x < 0.001)
			{
				++n001;
				if(pDm->x < 0.0001)
				{
					++n0001;
				}
			}
		}
		if(color == 0)
		{
			pDm->x = pDm->y = pDm->z = 0;
			++nBadNorm;
		}
 
		++pDm;	
		++pV;
		++pNorm;

		if(vIdx % 1000 == 0)
			cout<<vIdx<<" ";
	}
	//dump Dm
	size_t n = fwrite(&m_accessObj.m_pModel->vpDm[0], sizeof(Vec3), m_accessObj.m_pModel->nVertices + 1, f);
	assert(n == m_accessObj.m_pModel->nVertices + 1);
	fclose(f);
	cout<<endl<<"nBadNorm: "<<nBadNorm<<", <.01: "<<n01<<", <.001: "<<n001<<", <.0001:"<<n0001<<endl;
	m_bLoadedDm = true;
	glCullFace(GL_BACK);
	glViewport(0, 0, m_nWidth, m_nHeight);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	InitGL();
}

//
////depth : using a ray tracer by shooting a ray along the negative normal direction and detecting the intersection. used by SSSample integration upper limit
//void CPRTView::OnComputeSaveDm()
//{
//	printf("start compute save Dm...\n ");
//	if((!m_bLoadedObj))
//	{
//		printf("!m_bLoadedObj\n ");
//		return;
//	}
//
//	CString fnDm = m_filenameNoSuffix + ".Dm";
//	FILE* f;
//	if((f = fopen(((char*)(LPCSTR)fnDm), "wb")) == NULL)
//	{
//		printf("failed open fnDm \n ");
//		return;
//	}
//
//	assert(m_accessObj.m_pModel);
//	m_accessObj.m_pModel->vpDm.clear();
//	m_accessObj.m_pModel->vpDm.resize(m_accessObj.m_pModel->nVertices + 1);
//
//	glDrawBuffer(GL_BACK);
//	glReadBuffer(GL_BACK);
//
//	//Larger than the real range, so that verts on the bbox can be correctly handled
//	float maxCoord = (m_objBboxSize * 2.f);
//	glViewport(0, 0, maxCoord, maxCoord);  //(0,0,1,1)won't work, will die ugly
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_FRONT);
//	glEnable(GL_DEPTH_TEST);
//	glClearDepth(1.0);
//
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	//zNear, zFar 	The distances to the nearer and farther depth clipping planes. These distances are negative if the plane is to be behind the viewer. 
//	glOrtho(-maxCoord, maxCoord, -maxCoord, maxCoord, 0, maxCoord);	
//
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//
//	Vec3* pV = &m_accessObj.m_pModel->vpVertices[1]; //starts from 1;
//	Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[1];
//	Vec3* pDm = &m_accessObj.m_pModel->vpDm[1];
//	float depth = 0;
//	Vec3* pVert = NULL;
//	Vec3 vecX;
//	GLubyte color;
//	int nDmZero = 0, nBadNorm = 0;
//	for(unsigned int vIdx = 0; vIdx < m_accessObj.m_pModel->nVertices; ++vIdx)
//	{
//		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
//		vecX = Vec3(0.2f, 0.3f, 0.5f);
//		vecX.Normalize();
//		vecX = Cross(*pNorm, vecX);//center is in the viewport center, so up is random. but Up should not parellel norm
//		vecX.Normalize();
//		gluLookAt(pV->x - Epsilon * pNorm->x, pV->y - Epsilon * pNorm->y, pV->z - Epsilon * pNorm->z, 
//			pV->x - pNorm->x, pV->y - pNorm->y, pV->z - pNorm->z, vecX.x, vecX.y, vecX.z); //so that Center - Eye = -norm direction
//
//		glBegin(GL_TRIANGLES);
//		for(unsigned int i = 0; i < m_accessObj.m_pModel->nTriangles; i++)
//		{
//			for(int j = 0; j < 3;j++)
//			{
//				pVert = &m_accessObj.m_pModel->vpVertices[(m_accessObj.m_pModel->pTriangles[i]).vindices[j]];
//				glVertex3f(pVert->x, pVert->y, pVert->z);
//			}
//		}
//		glEnd();
//		glFlush();
//		glReadPixels(maxCoord/2, maxCoord/2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
//		glReadPixels(maxCoord/2, maxCoord/2, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &color);
//
//		if(color == 0)
//			++nBadNorm;
//		pDm->x = (color > 0) ? depth * maxCoord : 0;	//maxCoord * 2 is the Ortho size. queried depth of [0,1] should be scaled back by the ortho size
//		pDm->y = pDm->x;
//		pDm->z = pDm->x;
//		if(pDm->x < 0.0001)// Epsilon)
//			++nDmZero;
//
//		++pDm;	
//		++pV;
//		++pNorm;
//		
//		if(vIdx % 100 == 0)
//			printf("Dm of vert %d of %d vertices finished. \n",vIdx, m_accessObj.m_pModel->nVertices);
//	}
//	//dump Dm
//	size_t n = fwrite(&m_accessObj.m_pModel->vpDm[0], sizeof(Vec3), m_accessObj.m_pModel->nVertices + 1, f);
//	assert(n == m_accessObj.m_pModel->nVertices + 1);
//	fclose(f);
//
//	m_bLoadedDm = true;
//	glMatrixMode(GL_MODELVIEW);
//	glPopMatrix();
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	InitGL();
//	printf("nBadNorm: %d. nDmZero: %d. Compute save Dm finished \n ", nBadNorm, nDmZero);
//}

void CPRTView::OnDrawDm()
{
	m_bDrawDm = !m_bDrawDm;
	ChangeDrawElements();
}

void CPRTView::OnUpdateDrawDm(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawDm);
}

////////////////////////////////////////////////////////////////////////// 2-pass

//save .E, and also load into memory
void CPRTView::OnEditComputeSaveE()
{
	if((!m_bLoadedObj) || (!m_bLoadedVisibs))
	{
		printf("(!m_bLoadedObj) || !m_bLoadedVisibs \n ");
		return;
	}

	CString fnE = m_filenameNoSuffix + ".E";
	FILE* fE = NULL;
	if((fE = fopen(((char*)(LPCSTR)fnE), "wb")) == NULL)
	{
		printf("failed open fnE \n ");
		return;
	}

	//alloc Coefs in memory, no need to r/w memory
	m_accessObj.m_pModel->vpECoefs.clear();
	m_accessObj.m_pModel->vpECoefs.resize(m_accessObj.m_pModel->nVertices * m_ECoefNum);
	m_accessObj.m_pModel->vpECoefIdx.clear();
	m_accessObj.m_pModel->vpECoefIdx.resize(m_accessObj.m_pModel->nVertices * m_ECoefNum);

	int nVisPerside = m_cubeRes * m_cubeRes;
	vector<float> pOneE(_T6k, 0.f);
	Vec3 curDir;
	Vec3* pNorm = &m_accessObj.m_pModel->vpVertexNormals[0];
	byte* pInvis = &m_accessObj.m_pModel->vpVertInvisibs[0];
	float solidAngle = 1.f;
	float Cos = 0;
	for(unsigned int xiIdx = 0; xiIdx < m_accessObj.m_pModel->nVertices; ++xiIdx)
	{
		++pNorm;
		for(int side = 0; side < 6; ++side)
		{
			for(int offset = 0; offset < nVisPerside; ++offset)
			{
				if(*pInvis == 0)//not occluded
				{
					curDir = m_cubeMap.Offset2Dir(side, offset, m_cubeRes, solidAngle);
					Cos = max(Dot((*pNorm), curDir), 0);
					if(_Fwi1)
						pOneE[side * nVisPerside + offset] = Cos;
					else
						pOneE[side * nVisPerside + offset] = Cos * FresnelT(Cos);
				}
				else
					pOneE[side * nVisPerside + offset] = 0;
				++pInvis;
			}
			//wavelet E
			m_wavelet.NonstandDecomposition(&pOneE[side * _T1k], m_cubeRes, m_cubeRes);
		}
		//return into vpECoefs and vpECoefIdx
		if(_AreaWeightedHaarNoSort)
			CubeKeepLoss(&pOneE[0], m_cubeRes, m_ECoefNum, &m_accessObj.m_pModel->vpECoefs[xiIdx  * m_accessObj.m_pModel->nECoefNum], &m_accessObj.m_pModel->vpECoefIdx[xiIdx * m_accessObj.m_pModel->nECoefNum], false, false, false);
		else
			KeepLoss(&pOneE[0], _T6k, m_ECoefNum, &m_accessObj.m_pModel->vpECoefs[xiIdx  * m_accessObj.m_pModel->nECoefNum], &m_accessObj.m_pModel->vpECoefIdx[xiIdx * m_accessObj.m_pModel->nECoefNum]);

		size_t n = fwrite(&m_accessObj.m_pModel->vpECoefs[xiIdx * m_accessObj.m_pModel->nECoefNum], sizeof(float), m_ECoefNum, fE);
		assert(n == size_t(m_ECoefNum));

		n = fwrite(&m_accessObj.m_pModel->vpECoefIdx[xiIdx * m_accessObj.m_pModel->nECoefNum], sizeof(short int), m_ECoefNum, fE);
		assert(n == size_t(m_ECoefNum));

		if(xiIdx % 100 == 0)
			printf("E Coefs of vert %d of %d vertices finished.\n", xiIdx, m_accessObj.m_pModel->nVertices);
	}
	printf("E Coefs and Indices finished save.\n");
	fclose(fE);
}

//build m_tree only of Pv, Av. and Ev, which is ECoefNum.
void CPRTView::OnBuildTree()
{
	if(!m_bLoadedObj)
	{
		printf("(!m_bLoadedObj)\n ");
		return;
	}
	CString eName = m_filenameNoSuffix + ".E";
	if(!m_accessObj.LoadECoefs((char*)(LPCSTR)eName))	
	{	
		printf("failed LoadECoefs!\n ");
		return;
	}
	
	printf("start calc areas...\n");
	m_accessObj.CalcVertArea();

	m_pECoefsTree = new CKdTree(m_accessObj.m_pModel->nVertices, m_ECoefNum);
	for(unsigned int i = 0; i < m_accessObj.m_pModel->nVertices; ++i)
		m_pECoefsTree->pVIndices[i] = i + 1; //vpVertices starts from 1

	m_pECoefsTree->pP = NULL;
	m_pECoefsTree->pBox[0] = m_pECoefsTree->pBox[2] = m_pECoefsTree->pBox[4] = m_objBboxSize / 2.f;	//xMax, yMax, zMax
	m_pECoefsTree->pBox[1] = m_pECoefsTree->pBox[3] = m_pECoefsTree->pBox[5] = -m_objBboxSize / 2.f; //xMin, yMin, zMin
	m_pECoefsTree->depth = 0;
	
	float* pBufOneE = new float[_T6k];

	printf("start building mesh kd-tree...\n");

	RecurAccumETree(m_pECoefsTree, pBufOneE);
	
	delete[] pBufOneE;
	printf("Build mesh kd-tree finished.\n");
	//can only debug small mesh. box passed.
	if(m_accessObj.m_pModel->nVertices <= 100)
		m_pECoefsTree->PrintInfo();
}

//use _T6k pBufOneE to accum all pt's v[nVIndices]' ECoefs
void CPRTView::RecurAccumETree(CKdTree* pt, float* pBufOneE)
{
	for(unsigned int i = 0; i < pt->nVIndices; ++i)
	{
		memset(pBufOneE, 0, sizeof(float) * _T6k);
		//Ev
		int vIdxFrom0 = pt->pVIndices[i] - 1;
		for(unsigned int j = 0; j < m_ECoefNum; ++j)
		{
			int EIdx = m_accessObj.m_pModel->vpECoefIdx[vIdxFrom0 * m_accessObj.m_pModel->nECoefNum + j];
			pBufOneE[EIdx] += m_accessObj.m_pModel->vpECoefs[vIdxFrom0 * m_accessObj.m_pModel->nECoefNum + j];
		}
		//Pv
		pt->Pv += m_accessObj.m_pModel->vpVertices[vIdxFrom0 + 1];
		//Av
		pt->Av += m_accessObj.m_pModel->vpVertAreas[vIdxFrom0];
	}
	if(_AreaWeightedHaarNoSort)
		CubeKeepLoss(pBufOneE, m_cubeRes, m_ECoefNum, pt->pECoefs, pt->pECoefIdx, false, false, false);
	else
		KeepLoss(pBufOneE, _T6k, m_ECoefNum, pt->pECoefs, pt->pECoefIdx);

	pt->Pv = pt->Pv / (float)pt->nVIndices;
	//bug###pt->Av /= (float)pt->nVIndices;

	//split
	if(pt->nVIndices > (unsigned int)m_leafSampleNum)
	{
		pt->pL = new CKdTree((pt->nVIndices + 1) / 2, m_ECoefNum);	//if pt->vecCnt is odd, say 5, then pL gets 1 more than pR, 3:2
		pt->pR = new CKdTree(pt->nVIndices / 2, m_ECoefNum);
		pt->pL->pP = pt;
		pt->pR->pP = pt;
		pt->pL->depth = pt->pR->depth = pt->depth + 1;

		//has alloced pt->pL and pR, now find nth(pt->VIds, DIM(pt->depth)), fill VIndices and Boxes of pL and pR
		KdNodeSplit(pt);

		//recursive
		RecurAccumETree(pt->pL, pBufOneE);
		RecurAccumETree(pt->pR, pBufOneE);
	}
	else	//leaf
	{
		pt->pL = pt->pR = NULL;	
	}
	//8 vert box test passed
}

//has alloced pt->pL and pR, now find nth(pt->VIds, DIM(pt->depth)), fill VIndices and Boxes of pL and pR
void CPRTView::KdNodeSplit(CKdTree* pt)
{
	std::vector<unsigned int> PIds(pt->nVIndices);
	for(unsigned int i = 0; i < pt->nVIndices; ++i)
		PIds[i] = pt->pVIndices[i];

	//below split pt's vindices into 2 parts, pL and pR. And meantime calc bbox for both.
	memcpy(pt->pL->pBox, pt->pBox, sizeof(float) * 6);
	memcpy(pt->pR->pBox, pt->pBox, sizeof(float) * 6);
	switch(pt->depth % 3)
	{
	case 0:
		std::nth_element(PIds.begin(), PIds.begin() + pt->pL->nVIndices, PIds.end(), xLess(m_accessObj.m_pModel->vpVertices));
		pt->pL->pBox[0] = pt->pR->pBox[1] = m_accessObj.m_pModel->vpVertices[PIds[pt->pL->nVIndices]].x;
		break;
	case 1:
		std::nth_element(PIds.begin(), PIds.begin() + pt->pL->nVIndices, PIds.end(), yLess(m_accessObj.m_pModel->vpVertices));
		pt->pL->pBox[2] = pt->pR->pBox[3] = m_accessObj.m_pModel->vpVertices[PIds[pt->pL->nVIndices]].y;
		break;
	case 2:
		std::nth_element(PIds.begin(), PIds.begin() + pt->pL->nVIndices, PIds.end(), zLess(m_accessObj.m_pModel->vpVertices));
		pt->pL->pBox[4] = pt->pR->pBox[5] = m_accessObj.m_pModel->vpVertices[PIds[pt->pL->nVIndices]].z;
		break;
	default:
		break;
	}

	for(unsigned int i = 0; i < pt->pL->nVIndices; ++i)//pL
	{
		pt->pL->pVIndices[i] = PIds[i];
	}
	for(unsigned int i = 0; i < pt->pR->nVIndices; ++i)//pR
	{
		pt->pR->pVIndices[i] = PIds[pt->pL->nVIndices + i];
	}	

	//need test
}

//2pass, compute E-kd tree, and compute Td, save to .Td2
void CPRTView::OnMSTrans2Pass()
{
	//repulsion(uint* sampleVertIndices); -> omit currently
	//ECoefsKdTree(KdTree* t) by bbox, its key is 6k waveles, save to .kd;
	//ECoefs(sampleVIndices, useENum), dump to .EC; -> now we don't sample(repultion retiling): we use vId, ECoef[nv][useENum]
	//Trans(t), save to .TdC;

	//alloc oneT[6*32*32]
	//for each xo in nv
	//	CalcT(oneT, xoIdx, tree, eps);

	if((!m_bLoadedObj) || (!m_pECoefsTree))
	{
		printf("(!m_bLoadedObj) || !m_pECoefsTree \n ");
		return;
	}

	CString fnTd = m_filenameNoSuffix + ".Td2";
	FILE* fTd = NULL;
	if((fTd = fopen(((char*)(LPCSTR)fnTd), "wb")) == NULL)
	{
		printf("failed open writing .Td2 \n ");
		return;
	}

	Vec3* pBufOneTd = new Vec3[_T6k];
	Vec3* OneTdLoss = new Vec3[m_TdCoefNum];
	short int* OneTdIdxLoss = new short int[m_TdCoefNum];
	for(unsigned int xoIdx = 0; xoIdx < m_accessObj.m_pModel->nVertices; ++xoIdx)
	{
		memset(pBufOneTd, 0, sizeof(Vec3) * _T6k); //can't place it into Recursive, or it will be constantly ZERO!!!! 

		RecurAccumTd(pBufOneTd, &m_accessObj.m_pModel->vpVertices[xoIdx + 1], m_pECoefsTree);

		if(_AreaWeightedHaarNoSort)
			WeightedCubeKeepLossNoSort(pBufOneTd, m_cubeRes, m_TdCoefNum, OneTdLoss, OneTdIdxLoss);
		else
			KeepLoss(pBufOneTd, _T6k, m_TdCoefNum, OneTdLoss, OneTdIdxLoss);

		size_t n = fwrite(OneTdLoss, sizeof(Vec3), m_TdCoefNum, fTd);
		assert(n == size_t(m_TdCoefNum));
		n = fwrite(OneTdIdxLoss, sizeof(short int), m_TdCoefNum, fTd);
		assert(n == size_t(m_TdCoefNum));

		if(xoIdx % 100 == 0)
			printf("Td2 Coefs of vert %d of %d vertices finished.\n", xoIdx, m_accessObj.m_pModel->nVertices);
	}
	printf("Td2 Coefs and Indices finished save.\n");
	fclose(fTd);

	delete[] pBufOneTd;
	delete[] OneTdLoss;
	delete[] OneTdIdxLoss;
}

inline bool VInBox(Vec3* pV, float* pBox)
{
	return(	(pV->x <= pBox[0]) &&
		(pV->x >= pBox[1]) &&
		(pV->y <= pBox[2]) &&
		(pV->y >= pBox[3]) &&
		(pV->z <= pBox[4]) &&
		(pV->z >= pBox[5])	);
}

//accumulate Rd*E to Td, use pBuf as buf so to avoid frequent new/delete
void CPRTView::RecurAccumTd(Vec3* pBufOneTd, Vec3* pVert, CKdTree* pt)
{
	if(pt->pL)
	{
		if(!VInBox(pVert, pt->pBox))
		{
			float r = Distance(*pVert, pt->Pv);
			if(pt->Av / (r * r) < m_epsMSTdKd)	//distant voxel
			{
				Vec3 Rd1 = Rd(r);
				//accum
				for(unsigned int i = 0; i < m_ECoefNum; ++i)
					pBufOneTd[pt->pECoefIdx[i]] += Rd1 * pt->pECoefs[i]; 
				return;
			}
			else
			{
				RecurAccumTd(pBufOneTd, pVert, pt->pL);
				RecurAccumTd(pBufOneTd, pVert, pt->pR);
			}
		}
		else	//v in voxel
		{
			RecurAccumTd(pBufOneTd, pVert, pt->pL);
			RecurAccumTd(pBufOneTd, pVert, pt->pR);
		}
	}
	else //leaf
	{
		float r = Distance(*pVert, pt->Pv);
		Vec3 Rd1 = Rd(r);
		//accum
		for(unsigned int i = 0; i < m_ECoefNum; ++i)
			pBufOneTd[pt->pECoefIdx[i]] += Rd1 * pt->pECoefs[i];
	}
}
void CPRTView::OnLoadTd()
{
	//CString eName = m_filenameNoSuffix + ".E";
	//if(m_accessObj.LoadECoefs((char*)(LPCSTR)eName))
	//	;

	CString TdName = m_filenameNoSuffix + ".Td2";
	if(m_accessObj.LoadTd2Coefs((char*)(LPCSTR)TdName))	
	{
		m_bLoadedTdCoefs = true;
	}

	if(m_bLoadedTdCoefs && m_bLoadedCubeMap)
		m_bDrawMS = true;
	ChangeDrawElements();

}

void CPRTView::OnLoadT1Coefs()
{
	if(m_accessObj.LoadT1Coefs(m_filenameNoSuffix))	
	{
		m_bLoadedT1Coefs = true;
	}

	if(m_bLoadedT1Coefs && m_bLoadedCubeMap)
		m_bDrawSS = true;
	
	ChangeDrawElements();
}

//HG(Cos, g)
inline float CPRTView::HG(float Cos)
{
	return (1 - m_g * m_g)	/ pow(1 + m_g * m_g - 2 * m_g * Cos, 1.5f);
}

//generate 6*res*res wi and also rename it to wo, shape it to a HG(wi,wo) matrix, save to .hg, //so matlab could SVD it and get g[4][6k], h[4][6k]
//matlab can compute [u s v] = svd(mat(6*16*16, 6*16*16)) in about 5 minutes. no 4Pi.
//My HGmat's wi wo are both outward!!!
//save .hg: k->side->t->s, s is the most continous dimension
void CPRTView::ComputeSaveHGMat(int res)
{
	char str[5]; 
	itoa(res, str, 10);
	CString fn = "HG";
	fn = fn + str + "g";
	itoa((int)(m_g * 100), str, 10);
	fn = fn + str + ".hg";

	FILE* f;
	if((f = fopen(((char*)(LPCSTR)fn), "wb")) == NULL)
	{
		printf("failed open writing .hg.\n ");
		return;
	}

	float** HGmat = new float*[6*res*res];
	for(int i = 0; i < 6*res*res; ++i)
		HGmat[i] = new float[6*res*res];

	Vec3 wi, wo;
	float hg;

	//int sidewi,offsetwi,  sidewo, offsetwo;
	//for(int wiIdx = 0; wiIdx < 6*res*res; ++wiIdx)
	//{
	//	sidewi = wiIdx / (res*res);
	//	offsetwi = wiIdx % (res*res);
	//	wi = m_cubeMap.Offset2Dir(sidewi, offsetwi, res);
	//	for(int woIdx = wiIdx; woIdx < 6*res*res; ++woIdx)
	//	{
	//		sidewo = woIdx / (res*res);
	//		offsetwo = woIdx % (res*res);
	//		wo = m_cubeMap.Offset2Dir(sidewo, offsetwo, res);
	//		hg = HG(Dot(wi, wo));
	//		HGmat[wiIdx][woIdx] = hg;
	//		HGmat[woIdx][wiIdx] = hg;
	//	}
	//}
	for(int sidewi = 0; sidewi < 6; ++sidewi)
		for(int offsetwi = 0; offsetwi < res*res; ++offsetwi)
		{
			wi = m_cubeMap.Offset2Dir(sidewi, offsetwi, res);

			for(int sidewo = 0; sidewo < 6; ++sidewo)
				for(int offsetwo = 0; offsetwo < res*res; ++offsetwo)
				{
					if(sidewo * res * res + offsetwo < sidewi * res * res + offsetwi)
						continue;
					
					wo = m_cubeMap.Offset2Dir(sidewo, offsetwo, res);

					hg =  HG(-Dot(wi, wo)); //reason: my wi, wo are both outward, but func HG() requires the angle of INWARD wi and outward wo.So cos(pi-<wi,wo>) = -cos(<wi,wo>)
					HGmat[sidewi * res * res + offsetwi][sidewo * res * res + offsetwo] = hg;
					HGmat[sidewo * res * res + offsetwo][sidewi * res * res + offsetwi] = hg;
				}			
		}
	
	for(int i = 0; i < 6*res*res; ++i)
	{
		size_t n = fwrite(HGmat[i], sizeof(float), 6*res*res, f);
		assert(n ==  (size_t)6*res*res);
	}
	fclose(f);

	for(int i = 0; i < 6*res*res; ++i)
		if(HGmat[i]) delete[] HGmat[i];
	delete[] HGmat;

}


void CPRTView::OnUsephaseterm1()
{
	if(m_phaseSVDterm >= 1)
	{
		m_usePhaseTerm = 1;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm1(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 1)
		pCmdUI->SetCheck(m_usePhaseTerm == 1);
}

void CPRTView::OnUsephaseterm2()
{
	if(m_phaseSVDterm >= 2)
	{
		m_usePhaseTerm = 2;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm2(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 2)
		pCmdUI->SetCheck(m_usePhaseTerm == 2);
}

void CPRTView::OnUsephaseterm3()
{
	if(m_phaseSVDterm >= 3)
	{
		m_usePhaseTerm = 3;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm3(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 3)
		pCmdUI->SetCheck(m_usePhaseTerm == 3);
}


void CPRTView::OnUsephaseterm4()
{
	if(m_phaseSVDterm >= 4)
	{
		m_usePhaseTerm = 4;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm4(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 4)
		pCmdUI->SetCheck(m_usePhaseTerm == 4);
}

void CPRTView::OnUsephaseterm8()
{
	if(m_phaseSVDterm >= 8)
	{
		m_usePhaseTerm = 8;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm8(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 8)
		pCmdUI->SetCheck(m_usePhaseTerm == 8);
}

void CPRTView::OnUsephaseterm16()
{
	if(m_phaseSVDterm >= 16)
	{
		m_usePhaseTerm = 16;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm16(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 16)
		pCmdUI->SetCheck(m_usePhaseTerm == 16);
}

void CPRTView::OnUsephaseterm32()
{
	if(m_phaseSVDterm >= 32)
	{
		m_usePhaseTerm = 32;
		CalcLightCoef();
		CalcColor();
		Invalidate(); 
	}
}

void CPRTView::OnUpdateUsephaseterm32(CCmdUI *pCmdUI)
{
	if(m_phaseSVDterm >= 32)
		pCmdUI->SetCheck(m_usePhaseTerm == 32);
}

void CPRTView::OnSpec()
{
	_Spec = !_Spec;
	CalcLightCoef();
	CalcColor();
	Invalidate(); 
}

void CPRTView::OnUpdateSpec(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(_Spec == 1);
}

void CPRTView::OnViewDrawtexture()
{
	_Tex = !_Tex;
	ChangeDrawElements();
}

void CPRTView::OnUpdateViewDrawtexture(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(_Tex == 1);
}

void CPRTView::OnFileOpentexture()
{
	if(!_Tex)
	{
		printf("!_Tex!\n");
		return;
	}
	char workdir[128];
	_getcwd( workdir, 128);	//save dir

	char szFilter[] = "bmp Files (*.bmp)|*.bmp|All Files (*.*)|*.*||";

	CFileDialog openDlg(TRUE,NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter,
		this);
	CString szFilepath;
	
	if(openDlg.DoModal() == IDOK)
	{
		szFilepath = openDlg.GetPathName();
		AUX_RGBImageRec* ppp = auxDIBImageLoad((LPCSTR)szFilepath);
		if(!ppp->data)
			return;
		
		m_texW = ppp->sizeX;
		m_texH = ppp->sizeY;
		m_textureImage.clear();
		m_textureImage.resize(m_texW * m_texH * 4);
		unsigned char* pCol = ppp->data;
		unsigned char* pColImg = &m_textureImage[0];
		for(INT64 i = 0; i < m_texW * m_texH * 3; ++i)
		{
			*pColImg = *pCol;
			if((i % 3) == 2)
			{
				++pColImg;
				*pColImg = 255;
			}
			++pColImg;
			++pCol;
		}		
		if (ppp->data)						// If Texture Image Exists
			free(ppp->data);				// Free The Texture Image Memory
		free(ppp);							// Free The Image Structure
	
		printf("Texture loaded. Now LinearTexture and VertTexcoords.\n");
		
		m_accessObj.LinearTexture();
		m_accessObj.VertTexcoords();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glDeleteTextures(1, &m_meshTex);
		glGenTextures(1, &m_meshTex);
		glBindTexture(GL_TEXTURE_2D, m_meshTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_texW, m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, &m_textureImage[0]);
		CalcLightCoef();
		CalcColor();
		ChangeDrawElements();
	}
	_chdir(workdir);	//restore dir
}
void CPRTView::OnEditLoadvsb()
{
	CString m_visibFilename = m_filenameNoSuffix + ".vsb";
	if(m_accessObj.LoadVisibs((char*)(LPCSTR)m_visibFilename))	
	{
		m_bLoadedVisibs = true;
	}
	ChangeDrawElements();
}

void CPRTView::OnEditLoadE()
{
	CString eName = m_filenameNoSuffix + ".E";
	if(m_accessObj.LoadECoefs((char*)(LPCSTR)eName))	
	{
		m_bLoadedECoefs = true;
	}
	if(m_bLoadedECoefs && m_bLoadedCubeMap)
		m_bDrawMS = true;
	ChangeDrawElements();

}

inline float HGg(float Cos, float g)
{
	float tmp = 1 + g * g - 2 * g * Cos;
	if(fabs(tmp) < 1e-5)	//0
		return 0;
	return (1 - g * g)	/ (tmp * sqrtf(tmp));
	//return (1 - g * g)	/ pow(1 + g * g - 2 * g * Cos, 1.5f);
}

//write 1 float min, 1 float range, and all items in Coef are quant8 as byte
void Quant8Write(vector<float>& Coef, FILE* f)
{
	vector<float> :: iterator iter = min_element(Coef.begin(), Coef.end(), mod_lesser);
	float fmin = *iter;
	iter = max_element(Coef.begin(), Coef.end(), mod_lesser);
	float frange = *iter - fmin;

	size_t n = fwrite(&fmin, sizeof(float), 1, f);
	assert(n == 1);
	n = fwrite(&frange, sizeof(float), 1, f);
	assert(n == 1);
	if(frange > 1.0e-5) //all ele in Coef are the same, can't devide range: /0. then all are set to min.
		for_each(Coef.begin(), Coef.end(), Quantize8addHalf(fmin, frange));//[0.5f, 255.5f]
	else
		for_each(Coef.begin(), Coef.end(), Set2Zero());//[0.5f, 255.5f]
	byte tmpb;
	for(iter = Coef.begin(); iter < Coef.end(); ++iter)
	{
		tmpb = static_cast<byte>(*iter);
		n = fwrite(&tmpb, sizeof(byte), 1, f);
		assert(n == size_t(1));
	}
}

//for each g
//	for each of P6k wo
//		calc P6k wi's HG.  (wi wo are both outward!!!)
//		Coef(P6k)
//		P6kIdx2SqIdx(Coef) repack into s/l/i/j/M order
//		Sum(P6k, Coef)
//		QuantSave(Coef), QuantSave(Sum)
void CPRTView::ComputeSaveHGCoefSum(const float* gArray, const int gNum)
{
	FILE* fHGc;
	if((fHGc = fopen("HGCoefSum.HGc", "wb")) == NULL)
	{
		printf("failed open writing HGCoefSum.HGc.\n ");
		return;
	}
	cout<<"Start compute save .HGc...\n";

	Vec3 wi, wo;
	vector<float>Coef(_G6k, 0.f);
	vector<float>SqCoef(_G6k, 0.f);
	vector<float>Sum(_GSumNum, 0.f); //precomputed psum_table
	for(int gIdx = 0; gIdx < gNum; ++gIdx)
	{
		for(int sidewo = 0; sidewo < 6; ++sidewo)
			for(int offsetwo = 0; offsetwo < _G1k; ++offsetwo)
			{
				float* pCoef = &Coef[0];
				wo = m_cubeMap.Offset2Dir(sidewo, offsetwo, _GRes);
				for(int sidewi = 0; sidewi < 6; ++sidewi)
				{
					for(int offsetwi = 0; offsetwi < _G1k; ++offsetwi)
					{
						wi = m_cubeMap.Offset2Dir(sidewi, offsetwi, _GRes);
						*pCoef =  HGg(-Dot(wo, wi), gArray[gIdx]); //reason: my wi, wo are both outward, but func HG() requires the angle of INWARD wi and outward wo.So cos(pi-<wi,wo>) = -cos(<wi,wo>)
						++pCoef;
					}
					m_wavelet.NonstandDecomposition(&Coef[sidewi * _G1k], _GRes, _GRes);
					//size_t n = fwrite(&Coef[sidewi * _G1k], sizeof(float), 1, fHGc); //scale
					//assert(n == 1);
				}
 				Repack2SqWithScale(&SqCoef[0], &Coef[0], _GRes, _GLevelNum);
				Quant8Write(SqCoef, fHGc);
				PSum(&Sum[0], &Coef[0], _GRes, _GLevelNum);
				Quant8Write(Sum, fHGc);
			}
	}
	fclose(fHGc);
	cout<<"Finished compute save .HGc!\n";
}
void CPRTView::OnSSEditAniso()
{
	m_bSSEditAniso = !m_bSSEditAniso;
	printf("m_colorCoef=%f\n", m_colorCoef);
	//precomp sumP; precomp sumT;
	CalcColor();
	Invalidate();
}

void CPRTView::OnUpdateSSEditAniso(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bSSEditAniso);
}

void CPRTView::OnLoadPhaseCoefs()
{
	CString HGcName = "HGCoefSum.HGc";
	int fh;
	if((fh = _open(HGcName, _O_RDONLY|_O_BINARY)) == -1)
	{
		printf("failed open .HGc file\n");
		return;
	}
	printf("Start loading HGc...\n");
	//format: for 10 g, for _G6k wi:
	//1float min, 1float range, G6k wiCoef in slijm order(has scale in each side's first); then
	//1float min, 1float range, GSumNum psum in slij order.
	int hgcSize = 10 * _G6k * 
		(sizeof(float) * 2 + sizeof(byte) * _G6k + sizeof(float) * 2 + sizeof(byte) * _GSumNum);
	m_pHGCoefSum.clear();
	m_pHGCoefSum.resize(hgcSize);

	size_t n = _read(fh, &m_pHGCoefSum[0], hgcSize);
	assert(n == (size_t)(hgcSize));
	printf("load .HGc successfully.\n");
	_close(fh);
	m_bLoadedHGCoefSum = true;
}

void CPRTView::SetSigmaSR(float newNum)
{
	m_sigmaS.x = newNum;
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetSigmaSG(float newNum)
{
	m_sigmaS.y = newNum;
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetSigmaSB(float newNum)
{
	m_sigmaS.z = newNum;
	CalcColor();
	Invalidate(); 
}

//search array to find left,right neibor of newNum, calc and return idx, weight
void LinearIplt(float newNum, float* array, int arraySize, int& idx1, float& w1, int& idx2, float& w2)
{
	for(int i = 1; i < arraySize; ++i)
	{
		if(array[i] >= newNum)
		{
			idx1 = i - 1;
			idx2 = i;
			w1 = (array[i] - newNum) / (array[i] - array[i - 1]);
			w2 = 1.f - w1;
			return;
		}
	}
	assert(0);
}

void CPRTView::SetSigmaTR(float newNum)
{
	m_sigmaT.x = newNum;
	LinearIplt(newNum, &m_tArray[0], _TItplNum, m_tIdx1[0], m_tW1[0], m_tIdx2[0], m_tW2[0]);
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetSigmaTG(float newNum)
{
	m_sigmaT.y = newNum;
	LinearIplt(newNum, &m_tArray[0], _TItplNum, m_tIdx1[1], m_tW1[1], m_tIdx2[1], m_tW2[1]);
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetSigmaTB(float newNum)
{
	m_sigmaT.z = newNum;
	LinearIplt(newNum, &m_tArray[0], _TItplNum, m_tIdx1[2], m_tW1[2], m_tIdx2[2], m_tW2[2]);
	CalcColor();
	Invalidate(); 
}

void CPRTView::SetGR(float newNum)
{
	m_SSEditG.x = newNum;
	//LinearIplt(newNum, &m_gArray[0], m_gItplNum, m_gIdx1[0], m_gW1[0], m_gIdx2[0], m_gW2[0]);
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetGG(float newNum)
{
	m_SSEditG.y = newNum;
	//LinearIplt(newNum, &m_gArray[0], m_gItplNum, m_gIdx1[1], m_gW1[1], m_gIdx2[1], m_gW2[1]);
	CalcColor();
	Invalidate(); 
}
void CPRTView::SetGB(float newNum)
{
	m_SSEditG.z = newNum;
	//LinearIplt(newNum, &m_gArray[0], m_gItplNum, m_gIdx1[2], m_gW1[2], m_gIdx2[2], m_gW2[2]);
	CalcColor();
	Invalidate(); 
}

//all scale 1000times based on mm-2, i.e., to *10 mm
void CPRTView::SetMatByIndex(int idx)
{
	//donnot handle absorbing mat!!
	if((idx == 9)||(idx == 10) || (idx == 11) || ((idx >= 13) && (idx <= 18)) || ((idx >= 20) && (idx <= 23)) || (idx == 26) || (idx ==27) || (idx == 34))
	{
		printf("Don't support absorbing mat!\n");
		return;
	}

	float sigmaTRarray[40] = {0.9126f,	1.075f,	1.1874f,	0.4376f,	0.19f,	0.1419f,	0.2434f,	0.4282f,	0.7359f,	0.7143f,	0.6433f,	0.1299f,	0.4009f,	0.1577f,	0.1763f,	0.7639f,	0.1486f,	0.0295f,	0.1535f,	0.16f,	0.7987f,	0.1215f,	0.27f,	0.55f,	0.2513f,	0.3609f,	0.0288f,	0.0217f,	0.3674f,	0.34f,	0.3377f,	0.24f,	0.2574f,	0.76f,	0.0795f,	0.5098f,	3.3623f,	3.3645f,	3.4063f,	3.3997f};
	float sigmaTGarray[40] = {1.0748f,	1.2213f,	1.3296f,	0.5115f,	0.26f,	0.1625f,	0.2719f,	0.5014f,	0.9172f,	1.1688f,	0.999f,	0.1283f,	0.4185f,	0.1748f,	0.237f,	1.6429f,	0.321f,	0.0663f,	0.3322f,	0.25f,	0.5746f,	0.2101f,	0.63f,	1.25f,	0.3517f,	0.38f,	0.071f,	0.0788f,	0.4527f,	0.58f,	0.5573f,	0.37f,	0.3536f,	0.8685f,	0.1759f,	0.6476f,	3.2929f,	3.3158f,	3.341f,	3.3457f};
	float sigmaTBarray[40] = {1.25f,	1.3941f,	1.4602f,	0.6048f,	0.35f,	0.274f,	0.4597f,	0.5791f,	1.0688f,	1.7169f,	1.442f,	0.1395f,	0.4324f,	0.3512f,	0.2913f,	1.9196f,	0.736f,	0.1521f,	0.7452f,	0.33f,	0.2849f,	0.4407f,	0.83f,	1.53f,	0.4305f,	0.5632f,	0.0952f,	0.1022f,	0.5211f,	0.88f,	1.0122f,	0.45f,	0.484f,	0.9363f,	0.278f,	0.7944f,	3.2193f,	3.2428f,	3.281f,	3.2928f};
	float sigmaSRarray[40] = {0.9124f,	1.0748f,	1.1873f,	0.2707f,	0.0916f,	0.1418f,	0.2433f,	0.4277f,	0.7352f,	0.0177f,	0.0058f,	0.0069f,	0.2392f,	0.003f,	0.0031f,	0.0053f,	0.0037f,	0.0027f,	0.0495f,	0.1425f,	0.0553f,	0.0201f,	0.0128f,	0.0072f,	0.1617f,	0.3513f,	0.0104f,	0.0028f,	0.2791f,	0.0798f,	0.1928f,	0.1235f,	0.0654f,	0.2485f,	0.0145f,	0.3223f,	0.2415f,	0.18f,	0.099f,	0.1018f};
	float sigmaSGarray[40] = {1.0744f,	1.2209f,	1.3293f,	0.2828f,	0.1081f,	0.162f,	0.2714f,	0.4998f,	0.9142f,	0.0208f,	0.0141f,	0.0089f,	0.2927f,	0.0047f,	0.0048f,	0.f,	0.0069f,	0.0055f,	0.0521f,	0.1723f,	0.0586f,	0.0243f,	0.0155f,	0.f,	0.1606f,	0.3669f,	0.0114f,	0.0032f,	0.289f,	0.0898f,	0.2132f,	0.1334f,	0.0882f,	0.2822f,	0.0162f,	0.3583f,	0.2762f,	0.1834f,	0.1274f,	0.1033f};
	float sigmaSBarray[40] = {1.2492f,	1.3931f,	1.4589f,	0.297f,	0.146f,	0.2715f,	0.4563f,	0.5723f,	1.0588f,	0.f,	0.f,	0.0089f,	0.3745f,	0.0069f,	0.0066f,	0.f,	0.0074f,	0.f,	0.0597f,	0.1928f,	0.0906f,	0.0323f,	0.0196f,	0.f,	0.1669f,	0.5237f,	0.0147f,	0.0033f,	0.3086f,	0.1073f,	0.2259f,	0.1305f,	0.1568f,	0.3216f,	0.0202f,	0.4148f,	0.3256f,	0.2281f,	0.1875f,	0.1611f};
	
	float gRarray[40] = {0.932f,	0.819f,	0.75f,	0.907f,	0.91f,	0.85f,	0.873f,	0.934f,	0.862f,	0.965f,	0.926f,	0.943f,	0.933f,	0.914f,	0.919f,	0.974f,	0.917f,	0.918f,	0.969f,	0.912f,	0.949f,	0.947f,	0.947f,	0.961f,	0.929f,	0.548f,	0.91f,	0.927f,	0.911f,	0.946f,	0.919f,	0.902f,	0.849f,	0.802f,	0.921f,	0.907f,	0.842f,	0.902f,	0.726f,	0.929f};
	float gGarray[40] = {0.902f,	0.797f,	0.714f,	0.896f,	0.907f,	0.853f,	0.858f,	0.927f,	0.838f,	0.972f,	0.979f,	0.953f,	0.933f,	0.958f,	0.943f,	9999.f,	0.956f,	0.966f,	0.969f,	0.905f,	0.95f,	0.949f,	0.951f,	9999.f,	0.929f,	0.545f,	0.905f,	0.935f,	0.896f,	0.946f,	0.918f,	0.902f,	0.843f,	0.793f,	0.919f,	0.894f,	0.865f,	0.825f,	0.82f,	0.91f};
	float gBarray[40] = {0.859f,	0.746f,	0.681f,	0.88f,	0.914f,	0.842f,	0.832f,	0.916f,	0.806f,	9999.f,	9999.f,	0.952f,	0.935f,	0.975f,	0.972f,	9999.f,	0.982f,	9999.f,	0.975f,	0.892f,	0.971f,	0.945f,	0.974f,	9999.f,	0.931f,	0.565f,	0.92f,	0.994f,	0.884f,	0.949f,	0.922f,	0.904f,	0.926f,	0.821f,	0.931f,	0.888f,	0.912f,	0.914f,	0.921f,	0.945f};
	m_sigmaT.x = sigmaTRarray[idx];
	m_sigmaT.y = sigmaTGarray[idx];
	m_sigmaT.z = sigmaTBarray[idx];
	m_sigmaS.x = sigmaSRarray[idx];
	m_sigmaS.y = sigmaSGarray[idx];
	m_sigmaS.z = sigmaSBarray[idx];
	m_SSEditG.x = gRarray[idx];
	m_SSEditG.y = gGarray[idx];
	m_SSEditG.z = gBarray[idx];

	//interpolate limit:
	if(idx==36)
	{
		m_sigmaT.x = .9126f;
		m_sigmaT.y = 1.0748f;
		m_sigmaT.z = 1.25f;
		m_sigmaS.x = .9124f;
		m_sigmaS.y = 1.0744f;
		m_sigmaS.z = 1.2492f;
		m_SSEditG.x = 0.7f;
		m_SSEditG.y = 0.8f;
		m_SSEditG.z = 0.9f;
	}
	if(idx==37)
	{
		m_sigmaT.x = 1.f;
		m_sigmaT.y = 1.f;
		m_sigmaT.z = 1.f;
		m_sigmaS.x = 0.99f;
		m_sigmaS.y = 0.99f;
		m_sigmaS.z = 0.99f;
		m_SSEditG.x = 0.94f;
		m_SSEditG.y = 0.945f;
		m_SSEditG.z = 0.943f;
	}
	if(idx==38)
	{
		m_sigmaT.x = 1.075f;	
		m_sigmaT.y = 1.2213f;
		m_sigmaT.z = 1.3941f;
		m_sigmaS.x = 1.0748f;
		m_sigmaS.y = 1.2209f;
		m_sigmaS.z = 1.3931f;
		m_SSEditG.x = 0.89f;//iso
		m_SSEditG.y = 0.89f;
		m_SSEditG.z = 0.89f;
	}
	if(idx==39)
	{
		m_sigmaT.x = 1.f;
		m_sigmaT.y = 1.f;
		m_sigmaT.z = 1.f;
		m_sigmaS.x = 0.99f;
		m_sigmaS.y = 0.99f;
		m_sigmaS.z = 0.99f;
		m_SSEditG.x = 0.8f;
		m_SSEditG.y = 0.8f;
		m_SSEditG.z = 0.8f;
	}
	LinearIplt(m_sigmaT.x, &m_tArray[0], _TItplNum, m_tIdx1[0], m_tW1[0], m_tIdx2[0], m_tW2[0]);
	LinearIplt(m_sigmaT.y, &m_tArray[0], _TItplNum, m_tIdx1[1], m_tW1[1], m_tIdx2[1], m_tW2[1]);
	LinearIplt(m_sigmaT.z, &m_tArray[0], _TItplNum, m_tIdx1[2], m_tW1[2], m_tIdx2[2], m_tW2[2]);
	//LinearIplt(m_SSEditG.x, &m_gArray[0], m_gItplNum, m_gIdx1[0], m_gW1[0], m_gIdx2[0], m_gW2[0]);
	//LinearIplt(m_SSEditG.y, &m_gArray[0], m_gItplNum, m_gIdx1[1], m_gW1[1], m_gIdx2[1], m_gW2[1]);
	//LinearIplt(m_SSEditG.z, &m_gArray[0], m_gItplNum, m_gIdx1[2], m_gW1[2], m_gIdx2[2], m_gW2[2]);
	TCHAR tchBuffer[MAX_PATH ]; 
	//below are hardcodes
	m_pDlgInfo->m_sigmaSR.SetPos(int(m_sigmaS.x * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaSR, 	_gcvt(m_sigmaS.x, 5, tchBuffer)); 
	m_pDlgInfo->m_sigmaSG.SetPos(int(m_sigmaS.y * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaSG, 	_gcvt(m_sigmaS.y, 5, tchBuffer)); 
	m_pDlgInfo->m_sigmaSB.SetPos(int(m_sigmaS.z * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaSB, 	_gcvt(m_sigmaS.z, 5, tchBuffer)); 

	m_pDlgInfo->m_sigmaTR.SetPos(int(m_sigmaT.x * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaTR, 	_gcvt(m_sigmaT.x, 5, tchBuffer)); 
	m_pDlgInfo->m_sigmaTG.SetPos(int(m_sigmaT.y * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaTG, 	_gcvt(m_sigmaT.y, 5, tchBuffer)); 
	m_pDlgInfo->m_sigmaTB.SetPos(int(m_sigmaT.z * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_SigmaTBtrue, 	_gcvt(m_sigmaT.z, 5, tchBuffer)); 
	
	//0~0.95 only
	m_SSEditG.x = min(m_SSEditG.x, 0.949f);
	m_SSEditG.y = min(m_SSEditG.y, 0.949f);
	m_SSEditG.z = min(m_SSEditG.z, 0.949f);
	m_pDlgInfo->m_GR.SetPos(int(m_SSEditG.x * 100 ));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_GR, 	_gcvt(m_SSEditG.x, 5, tchBuffer)); 
	m_pDlgInfo->m_GG.SetPos(int(m_SSEditG.y * 100 ));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_GG, 	_gcvt(m_SSEditG.y, 5, tchBuffer)); 
	m_pDlgInfo->m_GB.SetPos(int(m_SSEditG.z * 100));
	m_pDlgInfo->SetDlgItemText(IDC_STATIC_GB, 	_gcvt(m_SSEditG.z, 5, tchBuffer)); 

	CalcColor();
	if(m_bDrawSS)
		Invalidate(); 
}
void CPRTView::OnComputeSaveHGCoefSum()
{		
	ComputeSaveHGCoefSum(&m_gArray[0], 10);//0.0:0.1:0.9.  1.0 always gets zero. 0.0 is iso, to test 3p
}
