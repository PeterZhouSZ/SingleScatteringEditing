#pragma once

#ifndef _KDTREE_H
#define _KDTREE_H

class CKdTree
{
public:
	CKdTree(unsigned int nVIndices, int ECoefNum);
	~CKdTree(void);

	void		Destroy();

	void		PrintInfo();

	//void		DumpData(CKdTree* pNode, FILE* fKd);

	CKdTree*	pL;		//left child
	CKdTree*	pR;		//right
	CKdTree*	pP;		//parent
	int			depth;	//depth starts from 0
	float*		pBox;	//bounding box xMax,xMin,yMax,yMin,zMax,zMin
	unsigned int nVIndices;//vertex indices

	//Jensen02 related
	unsigned int* pVIndices;	//indices in CObject.vpVertices, itself starts from 0 ,but its reference in vpVerts starts from 1
	float*		pECoefs;		//content
	short int* pECoefIdx;	//Idx

	float		Av;		//area
	Vec3		Pv;		//average location
	int			m_EcoefNum;
};


//////////////////////////////////////////////////////////////////////////template demo
//template <typename T>
//struct treeNode {
//	treeNode(T e, struct treeNode<T>* l = NULL, struct treeNode<T>* r = NULL)	: element(e), left(l), right(r) {}
//	~treeNode()
//	{
//		if (this->left)	{delete this->left; this->left = NULL;}
//		if (this->right){delete this->right; this->right = NULL;}
//	}
//	T element;
//	struct treeNode<T>* left;
//	struct treeNode<T>* left;
//};
//
//template <typename T>
//struct huffmanNode : treeNode<T> {
//public:
//	huffmanNode(T e, int w, struct huffmanNode<T>* l = NULL, struct huffmanNode<T>* r = NULL) : treeNode<T>(e, l, r), weight(w) {}
//	int weight;
//};

#endif