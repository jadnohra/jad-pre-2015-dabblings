#include "Common.fxh"

float4x4 WvpXf;
//float4 Ambient;
texture2D DiffuseTex;
texture2D LightmapTex;

sampler DiffuseTexSampler = sampler_state
{
	Texture = (DiffuseTex);
	MipFilter = ANISOTROPIC;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler LighmapTexSampler = sampler_state
{
	Texture = (LightmapTex);
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	//MipFilter = ANISOTROPIC;
	//MinFilter = ANISOTROPIC;
	//MagFilter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VSI_AmbientTexturedLighmapped
{
    float4 position : POSITION0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
    float2 tex_lm : TEXCOORD1;
};

struct VSO_AmbientTexturedLighmapped
{
    float4 position : POSITION0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
    float2 tex_lm : TEXCOORD1;
};

struct PSO_AmbientTexturedLighmapped
{
    float4 color : COLOR0;
};


VSO_AmbientTexturedLighmapped VS_AmbientTexturedLighmapped(VSI_AmbientTexturedLighmapped input)
{
	VSO_AmbientTexturedLighmapped output;
	
	output.position = mul(input.position, WvpXf);
	output.color = input.color;
	output.tex = input.tex;
	output.tex_lm = input.tex_lm;

    return output;
}	

PSO_AmbientTexturedLighmapped PS_AmbientTexturedLighmapped(VSO_AmbientTexturedLighmapped input)
{
	PSO_AmbientTexturedLighmapped output;
	
	//output.color = Ambient;
	//output.color = float4(1.0, 0.0, 0.0, 1.0);
	//output.color = tex2D(DiffuseTexSampler, input.tex) * float4(input.color.rgb, 1.0);
	float4 diffuseCol = tex2D(DiffuseTexSampler, input.tex);
	float4 lightmapCol = tex2D(LighmapTexSampler, input.tex_lm);
	output.color = diffuseCol * (4.0 * lightmapCol);
	
    return output;
}	

technique TechniqueHLSL
{
	pass Pass1
    {
		//ColorOp[0] = Modulate4x;
    
        VertexShader = compile vs_1_1 VS_AmbientTexturedLighmapped();
        PixelShader = compile ps_1_1 PS_AmbientTexturedLighmapped();
    }
}
