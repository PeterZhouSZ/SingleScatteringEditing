// PRTDoc.h : interface of the CPRTDoc class
//


#pragma once

class CPRTDoc : public CDocument
{
protected: // create from serialization only
	CPRTDoc();
	DECLARE_DYNCREATE(CPRTDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CPRTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


