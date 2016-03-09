// DlgInfo.cpp : implementation file
//

#include "stdafx.h"
#include "PRT.h"
#include "DlgInfo.h"
#include ".\dlginfo.h"
#include "PRTView.h"

// CDlgInfo dialog
#define BUFFER MAX_PATH 

IMPLEMENT_DYNAMIC(CDlgInfo, CDialog)
CDlgInfo::CDlgInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInfo::IDD, pParent)
{
	m_pView = NULL;
	m_oldMatIdx = -1;
}

CDlgInfo::~CDlgInfo()
{
}

void CDlgInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_SIGMAAB, m_sigmaTB);
	DDX_Control(pDX, IDC_SLIDER_SIGMASR, m_sigmaSR);
	DDX_Control(pDX, IDC_SLIDER_SIGMASG, m_sigmaSG);
	DDX_Control(pDX, IDC_SLIDER_SIGMASB, m_sigmaSB);
	DDX_Control(pDX, IDC_SLIDER_SIGMAAR, m_sigmaTR);
	DDX_Control(pDX, IDC_SLIDER_SIGMAAG, m_sigmaTG);
	DDX_Control(pDX, IDC_COMBO_MAT, m_mat);
	DDX_Control(pDX, IDC_SLIDER_GR, m_GR);
	DDX_Control(pDX, IDC_SLIDER_GG, m_GG);
	DDX_Control(pDX, IDC_SLIDER_GB, m_GB);
}


BEGIN_MESSAGE_MAP(CDlgInfo, CDialog)
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMAAB, OnNMReleasedcaptureSliderSigmaTb)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMASR, OnNMReleasedcaptureSliderSigmasr)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMASG, OnNMReleasedcaptureSliderSigmasg)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMASB, OnNMReleasedcaptureSliderSigmasb)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMAAR, OnNMReleasedcaptureSliderSigmaTr)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SIGMAAG, OnNMReleasedcaptureSliderSigmaTg)
	ON_CBN_CLOSEUP(IDC_COMBO_MAT, OnCbnCloseupComboMat)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GR, OnNMReleasedcaptureSliderGr)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GG, OnNMReleasedcaptureSliderGg)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GB, OnNMReleasedcaptureSliderGb)
END_MESSAGE_MAP()


// CDlgInfo message handlers

BOOL CDlgInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sigmaSR.SetRange(0, 150);
	m_sigmaSR.SetLineSize(1);
	m_sigmaSR.SetPageSize(1);
	m_sigmaSR.SetPos(0);
	m_sigmaSR.SetTicFreq(1);

	m_sigmaSG.SetRange(0, 150);
	m_sigmaSG.SetLineSize(1);
	m_sigmaSG.SetPageSize(1);
	m_sigmaSG.SetPos(0);
	m_sigmaSG.SetTicFreq(1);

	m_sigmaSB.SetRange(0, 150);
	m_sigmaSB.SetLineSize(1);
	m_sigmaSB.SetPageSize(1);
	m_sigmaSB.SetPos(0);
	m_sigmaSB.SetTicFreq(1);

	m_sigmaTR.SetRange(0, 150);
	m_sigmaTR.SetLineSize(1);
	m_sigmaTR.SetPageSize(1);
	m_sigmaTR.SetPos(0);
	m_sigmaTR.SetTicFreq(1);

	m_sigmaTG.SetRange(0, 150);
	m_sigmaTG.SetLineSize(1);
	m_sigmaTG.SetPageSize(1);
	m_sigmaTG.SetPos(0);
	m_sigmaTG.SetTicFreq(1);

	m_sigmaTB.SetRange(0, 150);
	m_sigmaTB.SetLineSize(1);
	m_sigmaTB.SetPageSize(1);
	m_sigmaTB.SetPos(0);
	m_sigmaTB.SetTicFreq(1);

	m_GR.SetRange(0, 100);
	m_GR.SetLineSize(1);
	m_GR.SetPageSize(1);
	m_GR.SetPos(0);
	m_GR.SetTicFreq(1);

	m_GG.SetRange(0, 100);
	m_GG.SetLineSize(1);
	m_GG.SetPageSize(1);
	m_GG.SetPos(0);
	m_GG.SetTicFreq(1);

	m_GB.SetRange(0, 100);
	m_GB.SetLineSize(1);
	m_GB.SetPageSize(1);
	m_GB.SetPos(0);
	m_GB.SetTicFreq(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInfo::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgInfo::OnNMReleasedcaptureSliderSigmasr(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaSR.GetPos() * 0.01f;
	m_pView->SetSigmaSR(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaSR, 	_gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}

void CDlgInfo::OnNMReleasedcaptureSliderSigmasg(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaSG.GetPos() * 0.01f;
	m_pView->SetSigmaSG(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaSG, _gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}

void CDlgInfo::OnNMReleasedcaptureSliderSigmasb(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaSB.GetPos() * 0.01f;
	m_pView->SetSigmaSB(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaSB, 	_gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}
void CDlgInfo::OnNMReleasedcaptureSliderSigmaTr(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaTR.GetPos() * 0.01f;
	m_pView->SetSigmaTR(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaTR, _gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}
void CDlgInfo::OnNMReleasedcaptureSliderSigmaTg(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaTG.GetPos() * 0.01f;
	m_pView->SetSigmaTG(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaTG, 	_gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}
void CDlgInfo::OnNMReleasedcaptureSliderSigmaTb(NMHDR *pNMHDR, LRESULT *pResult)
{
	float val = m_sigmaTB.GetPos() * 0.01f;
	m_pView->SetSigmaTB(val);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_SigmaTBtrue, 	_gcvt(val, 5, tchBuffer)); 
	*pResult = 0;
}

void CDlgInfo::OnCbnCloseupComboMat()
{
	int nIndex = m_mat.GetCurSel();//0-39. actually can't select last 4, so 0-35
	if(nIndex != m_oldMatIdx)
	{
	m_pView->SetMatByIndex(nIndex);
		m_oldMatIdx = nIndex;
	}
}

void CDlgInfo::OnNMReleasedcaptureSliderGr(NMHDR *pNMHDR, LRESULT *pResult)
{
	int pos = m_GR.GetPos();
	float newVal = float(0.01f * pos);
	//0.5~0.95 only
	if(newVal < 0.5f)
	{
		newVal = 0.5f;
		m_GR.SetPos(newVal / 0.01f);
	}
	if(newVal > 0.95f)
	{
		newVal = 0.95f;
		m_GR.SetPos(newVal / 0.01f);
	}

	m_pView->SetGR(newVal);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_GR, _gcvt(newVal, 5, tchBuffer)); 
	*pResult = 0;
}

void CDlgInfo::OnNMReleasedcaptureSliderGg(NMHDR *pNMHDR, LRESULT *pResult)
{
	int pos = m_GG.GetPos();
	float newVal = float(0.01f * pos);
	//0.5~0.95 only
	if(newVal < 0.5f)
	{
		newVal = 0.5f;
		m_GG.SetPos(newVal / 0.01f);
	}
	if(newVal > 0.95f)
	{
		newVal = 0.95f;
		m_GG.SetPos(newVal / 0.01f);
	}

	m_pView->SetGG(newVal);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_GG, _gcvt(newVal, 5, tchBuffer)); 
	*pResult = 0;
}

void CDlgInfo::OnNMReleasedcaptureSliderGb(NMHDR *pNMHDR, LRESULT *pResult)
{
	int pos = m_GB.GetPos();
	float newVal = float(0.01f * pos);
	//0.5~0.95 only
	if(newVal < 0.5f)
	{
		newVal = 0.5f;
		m_GB.SetPos(newVal / 0.01f);
	}
	if(newVal > 0.95f)
	{
		newVal = 0.95f;
		m_GB.SetPos(newVal / 0.01f);
	}

	m_pView->SetGB(newVal);
	TCHAR tchBuffer[BUFFER]; 
	SetDlgItemText(IDC_STATIC_GB, _gcvt(newVal, 5, tchBuffer)); 
	*pResult = 0;
}
