uniform int side;
uniform sampler2D NiSiSampler;
uniform vec2 intrudepdf; //[0]: intrudes, [1]: pdf

const float e = 2.7182818284;
const vec4 eeee = vec4(e,e,e,e);
const float oneby255 = 0.00392157;

//opti:
const float m_yita = 1.3;
const int m_cubeRes = 32;
const int m_HGres = 16;
const float m_SSSampleNum = 8.0; 
const vec4 sigmat0123 = vec4(0.0, 0.1, 0.2, 0.3);
const vec4 sigmat4567 = vec4(0.4, 0.5, 0.6, 0.7);
const vec4 sigmat891011 = vec4(0.8, 0.9, 1.0, 1.1);
const vec4 sigmat12131415 = vec4(1.2, 1.3, 1.4, 1.5);
///opti

//[0,31] -> (-1,1)
float A(in float i)
{
	return  ((i + 0.5) * 2.0 / float(m_cubeRes) - 1.0);
}

vec3 Texcoord2Dir(in vec2 texCoord)
{
	vec2 offset = float(m_cubeRes) * texCoord;
	vec3 w = vec3(0);

	if(side == 0)
		w = vec3(1.0, -A(offset.t), -A(offset.s)); //note: add '-' before A(y) !!! that's all about flip Ni!!
	else if(side == 1)
		w = vec3(-1.0, -A(offset.t), A(offset.s));
	else if(side == 2)
		w = vec3(A(offset.s), 1.0, A(offset.t));
	else if(side == 3)
		w = vec3(A(offset.s), -1.0, -A(offset.t));
	else if(side == 4)
		w = vec3(A(offset.s), -A(offset.t), 1.0);
	else if(side == 5)
		w = vec3(-A(offset.s), -A(offset.t), -1.0);

	return normalize(w);
}
/*
float FresnelT(in float Cos)
{
	return  (Cos > 0.0) ? 2.0 * Cos / (Cos + sqrtf(m_yita * m_yita - 1.0 + Cos * Cos)) : 0.0;
}
float FresnelR(in float Cos)
{
	return 1 - FresnelT(Cos);
}*/

void main(void)
{
	vec2 texCoord = vec2(gl_TexCoord[0]);	//gk use this
	//fliplr
	vec2 texCoordS0 = texCoord;				//ni, vis, si uses this
	if(side == 0)
		texCoordS0.x = 1.0 - texCoord.x;
		
	vec3 Ni = texture2D(NiSiSampler, texCoordS0).rgb;

	gl_FragData[0] = gl_FragData[1] = gl_FragData[2] = gl_FragData[3] = vec4(0.0);

	vec3 Wi = Texcoord2Dir(texCoordS0);
	float Cos = dot(Ni, Wi);
	if(Cos > 0.0)
	{
		float Si1 = texture2D(NiSiSampler, texCoordS0).a * (Cos / sqrt(1.0 - (1.0 - Cos * Cos) / m_yita / m_yita));
		 
		//by outer controls, only draw at vis==1 places. so here needn't care about visb issue.
		gl_FragData[0] = pow(eeee, - sigmat0123     * (intrudepdf.x + Si1)) / intrudepdf.y / m_SSSampleNum;
		gl_FragData[1] = pow(eeee, - sigmat4567     * (intrudepdf.x + Si1)) / intrudepdf.y / m_SSSampleNum;
		gl_FragData[2] = pow(eeee, - sigmat891011   * (intrudepdf.x + Si1)) / intrudepdf.y / m_SSSampleNum;
		gl_FragData[3] = pow(eeee, - sigmat12131415 * (intrudepdf.x + Si1)) / intrudepdf.y / m_SSSampleNum;
	}
}

/*back:
		float* pGk = &pG[0];
		int multiple = m_cubeRes / m_HGres;
		Vec3* pT1 = &pOneT1[0];
		for(int k = 0; k < m_phaseSVDterm; ++k)
		{
			for(int side = 0; side < 6; ++side)
				for(int row = 0; row < m_cubeRes; ++row)
					for(int col = 0; col < m_cubeRes; ++col)
					{
						pGk = &pG[k * 6 * m_HGres * m_HGres + side * m_HGres * m_HGres + (row/multiple) * m_HGres + (col/multiple)];//wi->wi' approx
						*pT1 = DP(*pT1, m_sigmaS) * (*pGk) / (float)m_SSSampleNum;
						++pT1;
					}

	if(side <= 1)
		gl_FragColor = vec4(texture2D(VisSampler, vec2(gl_TexCoord[0])).rgb, 1.0);
	else if(side <= 3)
		gl_FragColor = vec4(texture2D(NiSiSampler, vec2(gl_TexCoord[0])).rgb, 1.0);
	else 
		gl_FragColor = vec4(texture2D(NiSiSampler, vec2(gl_TexCoord[0])).a, 0, 0, 1.0);


gl_FragColor = vec4(255 * texture2D(VisSampler, vec2(gl_TexCoord[0]))); //for 1, *255 is 1.02, don't *255 is 0.00392: if texture2D() =~ 0.004...(stencil==1)



uniform int m_cubeRes, side;
uniform sampler2D NiSiSampler, VisSampler;
uniform vec3 m_sigmaT;
uniform float m_yita, intrude, pdf;

const float e = 2.7182818284;
const vec3 eee = vec3(e,e,e);
const float oneby255 = 0.00392157;

//[0,31] -> (-1,1)
float A(in float i)
{
	return ((i + 0.5) * 2.0 / float(m_cubeRes) - 1.0);
}

vec3 Texcoord2Dir(in vec2 texCoord)
{
	vec2 offset = float(m_cubeRes) * texCoord;
	vec3 w;

	if(side == 0)
		w = vec3(1, -A(offset.s), A(offset.t));
	else if(side == 1)
		w = vec3(-1, -A(offset.s), A(offset.t));
	else if(side == 2)
		w = vec3(A(offset.t), 1, A(offset.s));
	else if(side == 3)
		w = vec3(A(offset.t), -1, -A(offset.s));
	else if(side == 4)
		w = vec3(A(offset.t), -A(offset.s), 1);
	else if(side == 5)
		w = vec3(-A(offset.t), -A(offset.s), -1);
	return normalize(w);
}

void main(void)
{
	vec2 texCoord = vec2(gl_TexCoord[0]);
	vec3 Ni = texture2D(NiSiSampler, texCoord).rgb;
	bool bSSVis = false;
	if(abs(texture2D(VisSampler, texCoord).r - 1.0) < 0.05)		//stencil == 1
		bSSVis = true;
	gl_FragColor = vec4(0.0); //can't delete!
	if(bSSVis)
	{
		vec3 Wi = Texcoord2Dir(texCoord);
		float Cos = dot(Ni, Wi);
		if(Cos > 0.0)
		{
			float Si = texture2D(NiSiSampler, texCoord).a;
			float Si1 = Si * (Cos / sqrt(1.0 - (1.0 - Cos * Cos) / m_yita / m_yita));
			gl_FragColor = vec4(pow(eee, - m_sigmaT * (intrude + Si1)) / pdf, 1.0);	//yk * FresnelT(Cos);
		}
	}		
}

//I don't know why Si, vis of side0 are the same as CPU, altho I didn't do anything.... anyway, go on...
void main(void)
{
	vec2 texCoord = vec2(gl_TexCoord[0]);
	//fliplr
	vec2 texCoordS0 = texCoord;
	if(side == 0)
		texCoordS0.x = 1.0 - texCoord.x;
		
	vec3 Ni = texture2D(NiSiSampler, texCoordS0).rgb;
	bool bSSVis = false;
	if(abs(texture2D(VisSampler, texCoordS0).r - oneby255) < 0.001)		//stencil == 1
		bSSVis = true;
	gl_FragData[0] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 0.0);

	if(bSSVis)
	{
		vec3 Wi = Texcoord2Dir(texCoordS0);
		float Cos = dot(Ni, Wi);
		if(Cos > 0.0)
		{
			float Si = texture2D(NiSiSampler, texCoordS0).a;
			float Si1 = Si * (Cos / sqrt(1.0 - (1.0 - Cos * Cos) / m_yita / m_yita));
			vec3 t1 = pow(eeee, - m_sigmaT * (intrude + Si1)) / pdf;	//yk * FresnelT(Cos)
			float g0 = textureCube(g0Sampler, Wi).r;
			float g1 = textureCube(g1Sampler, Wi).r;
			float g2 = textureCube(g2Sampler, Wi).r;
			float g3 = textureCube(g3Sampler, Wi).r;
			gl_FragData[0] = vec4(t1 * m_sigmaS * g0 / float(m_SSSampleNum), 1.0);
			gl_FragData[1] = vec4(t1 * m_sigmaS * g1 / float(m_SSSampleNum), 1.0);
			gl_FragData[2] = vec4(t1 * m_sigmaS * g2 / float(m_SSSampleNum), 1.0);
			gl_FragData[3] = vec4(t1 * m_sigmaS * g3 / float(m_SSSampleNum), 1.0);
		}
	}
}
*/