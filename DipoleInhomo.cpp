#include "StdAfx.h"
#include ".\dipoleinhomo.h"

#ifdef	SPT_TREE_ENABLE_STAT
template<UINT t_Dimension,typename t_DisType,typename t_SPTreeNode>
int CSpatialPartitionTree<t_Dimension,t_DisType,t_SPTreeNode>::TotalTreeNodes = 0;
#endif

extern float g_fDipoleThreshold;
#define DII_SPLIT_THRESHOLD	g_fDipoleThreshold

__forceinline float sqr(float i)
{
	return i*i;
}

void stophere()
{
	ASSERT(0);
}

template<UINT t_Dimension,typename t_DisType,typename t_SPTreeNode>
void CSpatialPartitionTree<t_Dimension,t_DisType,t_SPTreeNode>::BuildTree(t_SPTreeNode *pNodes,UINT	Count)
{
	Empty();

	////////////////////////////////////////////////
	//Determine initial bounding box
	memcpy(m_SPTreeRoot.Box.Min,pNodes,sizeof(m_SPTreeRoot.Box.Min));
	memcpy(m_SPTreeRoot.Box.Max,pNodes,sizeof(m_SPTreeRoot.Box.Max));

	t_SPTreeNode *p = &pNodes[1];
	t_SPTreeNode *end = &pNodes[Count];

	for(;p<end;p++)
	{
		for(int i=0;i<t_Dimension;i++)
		{
			if( (*((CPos *)p))[i]>m_SPTreeRoot.Box.Max[i])
			{
				m_SPTreeRoot.Box.Max[i] = (*((CPos *)p))[i];
				continue;
			}

			if( (*((CPos *)p))[i]<m_SPTreeRoot.Box.Min[i])
			{
				m_SPTreeRoot.Box.Min[i] = (*((CPos *)p))[i];
				continue;
			}
		}
	}

	//////////////////////////////////////////////////////////////
	//Add all dots
	p = pNodes;
	end = &pNodes[Count];

	for(;p<end;p++)
	{
		m_SPTreeRoot.AddDot((CPos *)p);
	}

#ifdef	SPT_TREE_ENABLE_STAT
	spt_report("	SPT Initial Nodes:%d\n",TotalTreeNodes);
#endif

	///////////////////////////////////////////////////////////////
	//Finalize, delete all empty leaves
	
	m_SPTreeRoot.Finalize();

#ifdef	SPT_TREE_ENABLE_STAT
	spt_report("	SPT Nodes after finalize:%d\n",TotalTreeNodes);
#endif
	

}

int	NodeCount = 0;

void CDipoleInhomogeneous::SetInputBuffer(CDipoleSamplePoint * pParam,UINT	DotCount)
{
	ASSERT(pParam);

	spt_report("\n	Input sample dot: %d\n",DotCount);

	m_OcTree.BuildTree(pParam,DotCount);    
}

void CDipoleInhomogeneous::UpdateOcTreeParamRecursively(CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode * pNode)
{
	int i;
	if(pNode->IsLeaf())
	{
		i = pNode->LeafNodeNextSlot;
	}
	else
	{
		for(i=0;i<8;i++)
		{
			if(pNode->Children[i])
				UpdateOcTreeParamRecursively((CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode *)pNode->Children[i]);
			else
				break;
		}
	}

	int	ChildCount = i;

	//Irrad is weight
	float	TotIrr[3];
	TotIrr[0] = TotIrr[1] = TotIrr[2] = 0.0;

	for(i=0;i<ChildCount;i++)
	{
		ASSERT(pNode->Children[i]);
		TotIrr[0] += pNode->Children[i]->Irrad[0];
		TotIrr[1] += pNode->Children[i]->Irrad[1];
		TotIrr[2] += pNode->Children[i]->Irrad[2];
	}

	CheckValue(TotIrr[0]);
	CheckValue(TotIrr[1]);
	CheckValue(TotIrr[2]);

	Copy3fv(pNode->Irrad,TotIrr);

	pNode->Pos[0] = pNode->Pos[1] = pNode->Pos[2] = 
	pNode->Albedo[0] = pNode->Albedo[1] = pNode->Albedo[2] = 
	pNode->Sigma_t[0] = pNode->Sigma_t[1] = pNode->Sigma_t[2] = 0.0;


#define Weighted_Accumulate(Channel,i)	\
		pNode->Pos[Channel] += pNode->Children[i]->Pos[Channel]*pNode->Children[i]->Irrad[Channel]; \
		pNode->Albedo[Channel] += pNode->Children[i]->Albedo[Channel]*pNode->Children[i]->Irrad[Channel]; \
		pNode->Sigma_t[Channel] += pNode->Children[i]->Sigma_t[Channel]*pNode->Children[i]->Irrad[Channel];
#define Nonweighted_Accumulate(Channel,i)	\
		pNode->Pos[Channel] += pNode->Children[i]->Pos[Channel]; \
		pNode->Albedo[Channel] += pNode->Children[i]->Albedo[Channel]; \
		pNode->Sigma_t[Channel] += pNode->Children[i]->Sigma_t[Channel];

	for(i=0;i<ChildCount;i++)
	{
		if(TotIrr[0]>EPS)
		{
            Weighted_Accumulate(0,i);
		}
		else
		{
			Nonweighted_Accumulate(0,i);	
			TotIrr[0] = (float)ChildCount;
		}

		if(TotIrr[1]>EPS)
		{
            Weighted_Accumulate(1,i);
		}
		else
		{
			Nonweighted_Accumulate(1,i);			
			TotIrr[1] = (float)ChildCount;
		}

		if(TotIrr[2]>EPS)
		{
            Weighted_Accumulate(2,i);
		}
		else
		{
			Nonweighted_Accumulate(2,i);			
			TotIrr[2] = (float)ChildCount;
		}
	}
#undef	Weighted_Accumulate
#undef	Nonweighted_Accumulate

#define Weighted_Normalization(Pos) \
		pNode->Pos[0] /= TotIrr[0]; \
		pNode->Pos[1] /= TotIrr[1]; \
		pNode->Pos[2] /= TotIrr[2];

	Weighted_Normalization(Pos);
	Weighted_Normalization(Albedo);
	Weighted_Normalization(Sigma_t);
#undef Weighted_Normalization

	CheckValue(pNode->Pos[0]);
	CheckValue(pNode->Pos[1]);
	CheckValue(pNode->Pos[2]);
}

//float DipoleContribution(float Albedo,float Sigma_t,float distance_sqr,float m_funA)
//{
//	float sig_s_reduced = Albedo*Sigma_t*LUMI_PARSE_FUNCTION_REDUCATION;
//	float sig_a = Sigma_t - Albedo*Sigma_t;
//	float m_Zr = 1/(sig_s_reduced+sig_a);
//	float m_Sigma_tr_reduced = sqrt(3*(sig_a)*(sig_s_reduced + sig_a));
//	float m_Albedo_reduced_divide_by_4pi = sig_s_reduced*m_Zr/(4*PI);
//
//	float Zv = m_Zr*m_funA;
//	float dv_sqr = distance_sqr + Zv*Zv;
//	float dr_sqr = distance_sqr + m_Zr*m_Zr;
//	float dv = sqrt(dv_sqr);
//	float dr = sqrt(dr_sqr);
//
//	return	m_Albedo_reduced_divide_by_4pi*(
//			m_Zr*(m_Sigma_tr_reduced+1/dr)*exp(-m_Sigma_tr_reduced*dr)/dr_sqr + 
//			Zv*(m_Sigma_tr_reduced+1/dv)*exp(-m_Sigma_tr_reduced*dv)/dv_sqr );
//}

void CDipoleInhomogeneous::EvalueRadianceRecursively(CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode * pNode)
{
	//Determine if can be approximated by this node
	{
		////if very dark dot, kick it
		//if( pNode->Irrad[0] < EPS || pNode->Irrad[1] < EPS || pNode->Irrad[2] < EPS )return;

		float	distance_sqr;
		float	nodeDis = pNode->DistanceSqr(EvalueRadiance_Pos3fv);
		if(pNode->TotalDots*AvgAreaPerDot/
					(distance_sqr = DistanceUnitSqr*nodeDis)
    				< DII_SPLIT_THRESHOLD )
		{
			EvalueRadiance_Radiance3fv[0]+=pNode->Contribution(distance_sqr,m_funA,0);
			EvalueRadiance_Radiance3fv[1]+=pNode->Contribution(distance_sqr,m_funA,1);			
			EvalueRadiance_Radiance3fv[2]+=pNode->Contribution(distance_sqr,m_funA,2);	
			CheckValue(EvalueRadiance_Radiance3fv[0]);
			CheckValue(EvalueRadiance_Radiance3fv[1]);
			CheckValue(EvalueRadiance_Radiance3fv[2]);
			return;
		}
	}

	CheckValue(pNode->Pos[0]);
	CheckValue(pNode->Pos[1]);
	CheckValue(pNode->Pos[2]);

	if(pNode->IsLeaf())
	{
		//if( pNode->Irrad[0] < EPS || pNode->Irrad[1] < EPS || pNode->Irrad[2] < EPS )return;
		for(int i=0;i<pNode->LeafNodeNextSlot;i++)
		{
			float distance_sqr = DistanceUnitSqr*pNode->Children[i]->DistanceSqr(EvalueRadiance_Pos3fv);

			EvalueRadiance_Radiance3fv[0]+=pNode->Children[i]->Contribution(distance_sqr,m_funA,0);
			EvalueRadiance_Radiance3fv[1]+=pNode->Children[i]->Contribution(distance_sqr,m_funA,1);
			EvalueRadiance_Radiance3fv[2]+=pNode->Children[i]->Contribution(distance_sqr,m_funA,2);
			CheckValue(EvalueRadiance_Radiance3fv[0]);
			CheckValue(EvalueRadiance_Radiance3fv[1]);
			CheckValue(EvalueRadiance_Radiance3fv[2]);
		}
	}
	else
	{
		for(int i=0;i<8;i++)
		{
			if(pNode->Children[i])
				EvalueRadianceRecursively((CSpatialPartitionTree<3,float,CDipoleSamplePoint>::CSPTreeNode *)pNode->Children[i]);	
			else
				break;
		}
	}
}

void CDipoleInhomogeneous::EvalueRadiance(float * pPos3fv,float * pRadiance3fv,float RefractionIndex,float area ,float unit )
{
	float Fdr = (0.71f-1.44f/RefractionIndex)/RefractionIndex+0.668f+0.0636f*RefractionIndex;
	m_funA = 1+4/3*(1+Fdr)/(1-Fdr);

	AvgAreaPerDot = area;
    DistanceUnitSqr = unit*unit;

	EvalueRadiance_Pos3fv = pPos3fv;
	EvalueRadiance_Radiance3fv = pRadiance3fv;

	pRadiance3fv[0] = pRadiance3fv[1] = pRadiance3fv[2] = 0.0f;
	EvalueRadianceRecursively(m_OcTree.GetRoot());
}

void CDipoleInhomogeneous::EvalueRadianceArray(float * pPos3fv,UINT PosStep,float * pRadiance3fv,UINT RadStep,UINT count, float RefractionIndex,float area ,float unit )
{
	LPBYTE pos = (LPBYTE)pPos3fv;
	LPBYTE rad = (LPBYTE)pRadiance3fv;

	float totrad = 0;

	for(UINT i=0;i<count;i++,pos+=PosStep,rad+=RadStep)
	{
		if(i%1000 == 0)spt_report("	Evaluing ... %d%%   Total Irr:%0.1f\r",i*100/count,totrad);

		if(BadValue3f(((float*)pos)))printf("bad evalue pos at %d\n",i);
		EvalueRadiance((float*)pos,(float*)rad,RefractionIndex,area,unit);
		if(BadValue3f(((float*)rad)))printf("bad evalue rad at %d\n pos=(%0.2f,%0.2f,%0.2f)",i,((float*)pos)[0],((float*)pos)[1],((float*)pos)[2]);

		totrad+= ((float*)rad)[0] + ((float*)rad)[1] + ((float*)rad)[2];
	}

	spt_report("	Evaluing ... 100%%\n\n");

	float AvgAlbedo = (m_OcTree.GetRoot()->Albedo[0] + m_OcTree.GetRoot()->Albedo[1] + m_OcTree.GetRoot()->Albedo[2])/1.0f; 

	float scale =(	m_OcTree.GetRoot()->Irrad[0] + 
					m_OcTree.GetRoot()->Irrad[1] + 
					m_OcTree.GetRoot()->Irrad[2] )/ ( totrad*AvgAlbedo );
	//float scale =0.05*(	m_OcTree.GetRoot()->Irrad[0] + 
	//					m_OcTree.GetRoot()->Irrad[1] + 
	//					m_OcTree.GetRoot()->Irrad[2] )/ totrad;

	spt_report("	Total Irradiance: %f;	Total Radiance: %f\n",
						(m_OcTree.GetRoot()->Irrad[0] + 
						m_OcTree.GetRoot()->Irrad[1] + 
						m_OcTree.GetRoot()->Irrad[2] )/3.0,
						totrad/3.0);

	spt_report("	Radiance scale ratio: %f",scale);

	rad = (LPBYTE)pRadiance3fv;
	for(UINT i=0;i<count;i++,rad+=RadStep)
	{
        ((float *)rad)[0] *= scale;
        ((float *)rad)[1] *= scale;
        ((float *)rad)[2] *= scale;
	}
}


#define	TANGENT_AZIMUTH	2.0f/3.0f
#define TANGENT_ELEVATION_SQR	1
#define TANGENT_ELEVATION	1
#define CSC__ELEVATION_SQR		2
#define CSC__ELEVATION			1.4142f

void CalculateDipoleSingleScattering(CDipoleSamplePoint*	pDots,
					 UINT DotsTotal,
					 float	unit_mm,
					 float* pRad,UINT	RadStep
					 )				
{
	ASSERT(pDots);
	ASSERT(pRad);

	float * pRadiance = pRad;

	int i=0;
	float	TotalEnergy = 0;
	{
		CDipoleSamplePoint* p = pDots;
		CDipoleSamplePoint* pend = &pDots[DotsTotal];
		while(p<pend)
		{
			TotalEnergy+=p->Irrad[0] + p->Irrad[1] + p->Irrad[2];
			p++;
		}
	}

	CDipoleSamplePoint* v = pDots;
	CDipoleSamplePoint* vend = &pDots[DotsTotal];
	for(int i=0;v<vend;v++,i++,pRad = (float *)&(((LPBYTE)pRad)[RadStep]))
	{
		float	Rad_SS_R = 0.0f;
		float	Rad_SS_G = 0.0f;
		float	Rad_SS_B = 0.0f;

		spt_report("SS: %d%%\r",i*100/DotsTotal);

		CDipoleSamplePoint* c = pDots;
		CDipoleSamplePoint* cend = &pDots[DotsTotal];

		for(;c<cend;c++)
		{
			float XY_Dis = sqr(v->Pos[0]-c->Pos[0])+sqr(v->Pos[1]-c->Pos[1]);

			int tot_ss = 0;
			////single scattering
			if(v->Pos[0] <= c->Pos[0] && v->Pos[1] <= c->Pos[1] )
			{

				float y_pos = TANGENT_AZIMUTH*(c->Pos[0] - v->Pos[0]) + v->Pos[1];
				if(y_pos+0.5f>c->Pos[1] && y_pos-0.5f<c->Pos[1])
				{
					tot_ss++;
					float	s = CSC__ELEVATION*sqrt(XY_Dis)*unit_mm;
					float	h = (s*TANGENT_ELEVATION + v->Pos[2])*unit_mm;
					
					Rad_SS_R += c->ContributionSingleScatterIncoming(h,0)*v->ContributionSingleScatterOutgoing(s,0);
					Rad_SS_G += c->ContributionSingleScatterIncoming(h,1)*v->ContributionSingleScatterOutgoing(s,1);
					Rad_SS_B += c->ContributionSingleScatterIncoming(h,2)*v->ContributionSingleScatterOutgoing(s,2);
				}
			}
		}

		pRad[0] = Rad_SS_R;
		pRad[1] = Rad_SS_G;
		pRad[2] = Rad_SS_B;
	}

	pRad = pRadiance;
	 float	* rend = (float *)&(((LPBYTE)pRadiance)[RadStep*DotsTotal]);

	float	tot = 0.0f;

	while(pRad<rend)
	{
		tot+=pRad[0]+pRad[1]+pRad[2];
		pRad = (float *)&(((LPBYTE)pRad)[RadStep]);
	}

	tot = TotalEnergy/tot*0.15f;
    
	pRad = pRadiance;
	while(pRad<rend)
	{
		pRad[0] *= tot;
		pRad[0] *= tot;
		pRad[0] *= tot;
		pRad = (float *)&(((LPBYTE)pRad)[RadStep]);
	}
}

