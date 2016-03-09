// PRTDoc.cpp : implementation of the CPRTDoc class
//

#include "stdafx.h"
#include "PRT.h"

#include "PRTDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPRTDoc

IMPLEMENT_DYNCREATE(CPRTDoc, CDocument)

BEGIN_MESSAGE_MAP(CPRTDoc, CDocument)
END_MESSAGE_MAP()


// CPRTDoc construction/destruction

CPRTDoc::CPRTDoc()
{
	// TODO: add one-time construction code here

}

CPRTDoc::~CPRTDoc()
{
}

BOOL CPRTDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CPRTDoc serialization

void CPRTDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CPRTDoc diagnostics

#ifdef _DEBUG
void CPRTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPRTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CPRTDoc commands
