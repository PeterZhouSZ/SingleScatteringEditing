#include "stdafx.h"
#include "3Dmath.h"

#include ".\kdtree.h"

CKdTree::CKdTree(unsigned int nvIndices, int ECoefNum)
{
	pBox = new float[6];
	memset(pBox, 0, sizeof(float) * 6);

	nVIndices = nvIndices;
	pVIndices = new unsigned int[nVIndices];
	memset(pVIndices, 0, sizeof(unsigned int) * nVIndices);

	pECoefs = new float[ECoefNum];
	memset(pECoefs, 0, sizeof(float) * ECoefNum);
	pECoefIdx = new short int[ECoefNum];
	memset(pECoefIdx, 0, sizeof(short int) * ECoefNum);

	m_EcoefNum = ECoefNum;
	
	Av = 0;
	Pv = Vec3(0,0,0);
	depth = 0;
	pL = pR = pP = NULL;
}

CKdTree::~CKdTree(void)
{
	delete pL;	//this will call ~pL() recursively, no need to call Destroy()
	delete pR;
	pL = pR = pP = NULL;

	delete[] pBox;
	pBox = NULL;

	if(pECoefs)delete[] pECoefs;
	pECoefs = NULL;
	
	delete[] pVIndices;
	pVIndices = NULL;

}

void CKdTree::Destroy()
{
	delete[] pBox;
	pBox = NULL;
	
	if(pECoefs)delete[] pECoefs;
	pECoefs = NULL;
	
	delete[] pVIndices;
	pVIndices = NULL;

	if(!pL)	//assert !pR, now is leaf.
	{
		delete[] pP;
		pP = NULL;
		return;
	}
    pL->Destroy();
	pR->Destroy();
	pL = pR = pP = NULL;
}

//void CKdTree::DumpData(CKdTree* pNode, FILE* fKd)
//{
//	size_t n = fwrite(pNode->pECoefs, sizeof(float), nECoefs, fKd);
//	assert(n == nECoefs);
//
//	if(pNode->pL == NULL)
//		return;
//    DumpData(pNode->pL, fKd);
//	DumpData(pNode->pR, fKd);
//}

void CKdTree::PrintInfo()
{
	float EvSum = 0;
	for(int i = 0; i < m_EcoefNum; ++i)
		EvSum += pECoefs[i];
        printf("\nPv=(%f,%f,%f), EvSum=%f, Av=%f\n", Pv.x, Pv.y, Pv.z, EvSum, Av);
	for(int i = 0; i < depth; ++i)
		printf("- ");
	for(unsigned int i = 0; i < nVIndices; ++i)
		printf("%d ",pVIndices[i]);

	if(pL == NULL)
		return;

	printf("\nL:");
	pL->PrintInfo();
	printf("\nR:");
	pR->PrintInfo();
}

