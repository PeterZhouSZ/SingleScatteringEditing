#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#ifndef _DlgInfo_H_
#define _DlgInfo_H_

class CPRTView;//dlg
// CDlgInfo dialog

class CDlgInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlgInfo)

public:
	CPRTView* m_pView;	//dlg
	CDlgInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInfo();

// Dialog Data
	enum { IDD = IDD_DIALOG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	CSliderCtrl m_sigmaTB;
	afx_msg void OnNMReleasedcaptureSliderSigmaTb(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_sigmaSR;
	CSliderCtrl m_sigmaSG;
	CSliderCtrl m_sigmaSB;
	CSliderCtrl m_sigmaTR;
	CSliderCtrl m_sigmaTG;
	afx_msg void OnNMReleasedcaptureSliderSigmasr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderSigmasg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderSigmasb(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderSigmaTr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderSigmaTg(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_mat;
	afx_msg void OnCbnCloseupComboMat();
	CSliderCtrl m_GR;
	CSliderCtrl m_GB;
	CSliderCtrl m_GG;
	afx_msg void OnNMReleasedcaptureSliderGr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderGb(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderGg(NMHDR *pNMHDR, LRESULT *pResult);
	int		m_oldMatIdx;	//old mat idx, to avoid reload same files
};

#endif
