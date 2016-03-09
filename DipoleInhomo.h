#pragma once

#include <math.h>
#include <float.h>


//The first component of t_SPTreeNode must be a CPos
//I will use something like CPos* p = (CPos*)&TreeNode;
#define CheckValue(x)	//if(_fpclass(x) <=0x2 || _fpclass(x) == _FPCLASS_PINF )printf("%s:%d " #x "\n",__FILE__,__LINE__)

#define BadValue(x)	0 //(_fpclass(x) <=0x2 || _fpclass(x) == _FPCLASS_PINF )
#define BadValue3f(x) 0 //( BadValue(x[0]) || BadValue(x[1]) || BadValue(x[2]) )
#define BadValue4f(x) 0 //( BadValue(x[0]) || BadValue(x[1]) || BadValue(x[2]) || BadValue(x[3]) )

extern float g_fDipoleThreshold;

#ifndef PI
	#define PI	3.14159265359f
#endif

#ifndef EPS
	#define EPS	1e-20f
#endif

#define spt_report	printf

#define	SPT_TREE_CHILDCOUNT	(1<<t_Dimension)
#define SPT_TREE_ENABLE_STAT
template<UINT t_Dimension,typename t_DisType,typename t_SPTreeNode>
class CSpatialPartitionTree
{
public:
	typedef t_DisType CPos[t_Dimension];
	//class CSPTreeSampleDot
	//{
	//	CPos		Pos;
	//	t_PowerType	Power;
	//	void		Combine(CSPTreeSampleDot * pDots)	//size_is SPT_TREE_CHILDCOUNT
	//};

#ifdef	SPT_TREE_ENABLE_STAT
	static int	TotalTreeNodes;
#endif

public:
	class CBoundingBox
	{
	public:
		CPos	Min;
		CPos	Max;

		BOOL	IsPtInBox(CPos	*p)
		{
			int i;
			for(i=0;i<t_Dimension;i++)
				if( Min[i]>(*p)[i] || Max[i]<(*p)[i] )return FALSE;
			return TRUE;
		}

		void	GetSubspace(int id,CBoundingBox & Subbox)		// i {- [0,SPT_TREE_CHILDCOUNT-1]
		{
			for(int j=0;j<t_Dimension;j++,(id>>=1))
			{
				if(id&1)
				{
					Subbox.Min[j] = (Min[j]+Max[j])/2;
					Subbox.Max[j] = Max[j];
				}
				else
				{
					Subbox.Min[j] = Min[j];
					Subbox.Max[j] = (Min[j]+Max[j])/2;
				}
			}
		}
	};

	class CSPTreeNode:public t_SPTreeNode
	{
	public:
		__forceinline	BOOL IsLeaf(){return LeafNodeNextSlot>=0;}
		int				TotalDots;
		int				LeafNodeNextSlot;					//if it <0, indicate a non-leaf node
		CBoundingBox	Box;								//Bound for this node
		t_SPTreeNode*	Children[SPT_TREE_CHILDCOUNT];		//if not leaf, point to a CSPTreeNode actually
	public:
		CSPTreeNode()
		{
			TotalDots = LeafNodeNextSlot = 0; 
			ZeroMemory(Children,SPT_TREE_CHILDCOUNT*sizeof(LPVOID));
		}
		~CSPTreeNode()
		{
			if(!IsLeaf())DeleteChildren();
		}

		void DeleteChildren()
		{
			ASSERT(!IsLeaf());
			int i;
			for(i=0;i<SPT_TREE_CHILDCOUNT;i++)
			{
				delete ((CSPTreeNode*)Children[i]);  //If not leaf we should delete as the type of CSPTreeNode or the destructor will not be called
				Children[i] = NULL;
			}
			_CheckHeap;
		}

		void Split()
		{
			ASSERT(IsLeaf());
			t_SPTreeNode*	old[SPT_TREE_CHILDCOUNT];
			memcpy(old,Children,LeafNodeNextSlot*sizeof(t_SPTreeNode*));

			int i;
            for(i=0;i<SPT_TREE_CHILDCOUNT;i++)
			{
				_CheckHeap;
				Children[i] = new CSPTreeNode;
				if(Children[i])
				{
					Box.GetSubspace(i,((CSPTreeNode*)Children[i])->Box);
				}
				else
					ASSERT(0);
			}

			int OldDotCount = LeafNodeNextSlot;
			LeafNodeNextSlot = -1;

            for(i=0;i<OldDotCount;i++)
			{
				AddDot((CPos *)old[i]);
			}

			TotalDots-=OldDotCount;

#ifdef	SPT_TREE_ENABLE_STAT
			TotalTreeNodes+=8;
#endif
		}

		void AddDot(CPos * p)
		{
			TotalDots++;

			if(IsLeaf())
			{
				if(LeafNodeNextSlot<SPT_TREE_CHILDCOUNT)
				{
					Children[LeafNodeNextSlot++] = (t_SPTreeNode*)p;
				}
				else
				{
					Split();
					goto CSPTreeNode_AddThisDotIntoChildNode;
				}
			}
			else
			{
CSPTreeNode_AddThisDotIntoChildNode:
                for(int i=0;i<SPT_TREE_CHILDCOUNT;i++)
				{
					if(((CSPTreeNode*)Children[i])->Box.IsPtInBox(p))
					{
						((CSPTreeNode*)Children[i])->AddDot(p);
						return;
					}
				}

				ASSERT(0);
			}
		}

		void Finalize()
		{
			if(IsLeaf())return;

			_CheckHeap;

			t_SPTreeNode ** pOpen = Children;
			for(int i=0;i<SPT_TREE_CHILDCOUNT;i++)
			{
				if( ((CSPTreeNode*)Children[i])->IsLeaf() && ((CSPTreeNode*)Children[i])->LeafNodeNextSlot == 0)
				{//Kill this node
					delete ((CSPTreeNode*)Children[i]);
					Children[i] = NULL;
					TotalTreeNodes--;
				}
				else
				{
					t_SPTreeNode * t = Children[i];
					Children[i] = NULL;
					*pOpen = t;

					((CSPTreeNode*)(*pOpen))->Finalize();
					pOpen++;
				}
			}
		}
	};

protected:
	CSPTreeNode	m_SPTreeRoot;
	
public:
	
	void Empty()
	{
		if(!m_SPTreeRoot.IsLeaf())m_SPTreeRoot.DeleteChildren();
		m_SPTreeRoot.LeafNodeNextSlot = 0;
		m_SPTreeRoot.TotalDots = 0;
		TotalTreeNodes = 1;
	}
	CSPTreeNode* GetRoot(){return &m_SPTreeRoot;} 

	void BuildTree(t_SPTreeNode *p,UINT	Count);
	~CSpatialPartitionTree(){ Empty(); }
};

#define DII_SPLIT_THRESHOLD	g_fDipoleThreshold//1.f
#define Copy3fv(x,y)	memcpy(x,y,sizeof(float)*3)
#define LUMI_PARSE_FUNCTION_REDUCATION	1.0f		//the (1-g) in paper, isotropic parse function by default


class CDipoleSamplePoint	
{
public:
	CDipoleSamplePoint(){ZeroMemory(this,sizeof(CDipoleSamplePoint));}
	float Pos[3];		//xyz
	float Irrad[3];		//rgb
	float Sigma_t[3];
	float Albedo[3];
	float DistanceSqr(float * pPos3fv) const
	{
		return	(pPos3fv[0] - Pos[0])*(pPos3fv[0] - Pos[0]) + 
				(pPos3fv[1] - Pos[1])*(pPos3fv[1] - Pos[1]) + 
				(pPos3fv[2] - Pos[2])*(pPos3fv[2] - Pos[2]);
	}
	float Contribution(float distance_sqr,float m_funA,int channel) const
	{
		if(Irrad[channel]>EPS)
		{
			float sig_s_reduced = Albedo[channel]*Sigma_t[channel]*LUMI_PARSE_FUNCTION_REDUCATION;
			float sig_a = Sigma_t[channel] - Albedo[channel]*Sigma_t[channel];
			float m_Zr = 1/(sig_s_reduced+sig_a);

			///////////////////////////////////////////////
			//Debugging
			if(Sigma_t[channel] < 9e-10)
			{
				printf("@@");
				AfxMessageBox("");
			}

			float m_Sigma_tr_reduced;
			{
				float t = 3*(sig_a)*(sig_s_reduced + sig_a);
				if(t>0)
				{
					m_Sigma_tr_reduced = sqrt(t);
				}
				else
				{
					m_Sigma_tr_reduced = 0;
				}
			}

			float m_Albedo_reduced_divide_by_4pi = sig_s_reduced*m_Zr/(4*PI);

			float Zv = m_Zr*m_funA;
			float dv_sqr = distance_sqr + Zv*Zv;
			float dr_sqr = distance_sqr + m_Zr*m_Zr;
			float dv = sqrt(dv_sqr);
			float dr = sqrt(dr_sqr);

			CheckValue(distance_sqr);
			CheckValue(m_funA);
			CheckValue(sig_s_reduced);
			CheckValue(sig_a);
			CheckValue(m_Zr);
			CheckValue(m_Sigma_tr_reduced);
			CheckValue(m_Albedo_reduced_divide_by_4pi);
			CheckValue(Zv);
			CheckValue(dv_sqr);
			CheckValue(dr_sqr);
			CheckValue(dv);
			CheckValue(dr);
			CheckValue((1/dv_sqr));
			CheckValue((1/dr_sqr));
			CheckValue((1/dv));
			CheckValue((1/dr));

			return	Irrad[channel]*m_Albedo_reduced_divide_by_4pi*(
					m_Zr*(m_Sigma_tr_reduced+1/dr)*exp(-m_Sigma_tr_reduced*dr)/dr_sqr + 
					Zv*(m_Sigma_tr_reduced+1/dv)*exp(-m_Sigma_tr_reduced*dv)/dv_sqr );
		}
		else
			return 0;
	}

	__forceinline float ContributionSingleScatterOutgoing(float distance,int channel) const
	{
		return exp(-Sigma_t[channel]*distance);
	}

	__forceinline float ContributionSingleScatterIncoming(float distance,int channel) const
	{
		return ContributionSingleScatterOutgoing(distance,channel)*Irrad[channel]*Albedo[channel]*Sigma_t[channel]*LUMI_PARSE_FUNCTION_REDUCATION;
	}

};

class CDipoleInhomogeneous
{
private:
	float	*	EvalueRadiance_Pos3fv;
	float	*	EvalueRadiance_Radiance3fv;
	float	m_funA;
	float	AvgAreaPerDot;
	float	DistanceUnitSqr;

protected:
	CSpatialPartitionTree<3,float,CDipoleSamplePoint>	m_OcTree;
	
protected:
	void UpdateOcTreeParamRecursively(CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode * pNode);
	void EvalueRadianceRecursively(CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode * pNode);

public:
	void SetInputBuffer(CDipoleSamplePoint * pParam,UINT	DotCount);
	void UpdateOcTreeParam()
	{
		UpdateOcTreeParamRecursively(m_OcTree.GetRoot());
		printf("	avg Abledo:%0.2f,%0.2f,%0.2f   Sigma_t:%0.2f,%0.2f,%0.2f\n	Irrad: %0.2f,%0.2f,%0.2f Pos: %0.2f,%0.2f,%0.2f\n",
					m_OcTree.GetRoot()->Albedo[0],m_OcTree.GetRoot()->Albedo[1],m_OcTree.GetRoot()->Albedo[2],
					m_OcTree.GetRoot()->Sigma_t[0],m_OcTree.GetRoot()->Sigma_t[1],m_OcTree.GetRoot()->Sigma_t[2],
					m_OcTree.GetRoot()->Irrad[0],m_OcTree.GetRoot()->Irrad[1],m_OcTree.GetRoot()->Irrad[2],
					m_OcTree.GetRoot()->Pos[0],m_OcTree.GetRoot()->Pos[1],m_OcTree.GetRoot()->Pos[2]);
	}
	void EvalueRadiance(float * pPos3fv,float * pRadiance3fv,float RefractionIndex,float area = 1,float unit = 1);
	void EvalueRadianceArray(float * pPos3fv,UINT PosStep,float * pRadiance3fv,UINT RadStep,UINT count, float RefractionIndex,float area = 1,float unit = 1);  //XXXStep is in byte
};

void CalculateDipoleSingleScattering(CDipoleSamplePoint*	pDots,
					 UINT DotsTotal,
					 float	unit_mm,
					 float* pRad,UINT	RadStep
					 );
