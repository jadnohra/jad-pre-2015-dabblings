#include "Common.fxh"

float4x4 WvpXf;
//float4 Ambient;
texture2D DiffuseTex;

sampler DiffuseTexSampler = sampler_state
{
	Texture = (DiffuseTex);
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VSI_AmbientTextured
{
    float4 position : POSITION0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

struct VSO_AmbientTextured
{
    float4 position : POSITION0;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

struct PSO_AmbientTextured
{
    float4 color : COLOR0;
};


VSO_AmbientTextured VS_AmbientTextured(VSI_AmbientTextured input)
{
	VSO_AmbientTextured output;
	
	output.position = mul(input.position, WvpXf);
	output.color = input.color;
	output.tex = input.tex;

    return output;
}	

PSO_AmbientTextured PS_AmbientTextured(VSO_AmbientTextured input)
{
	PSO_AmbientTextured output;
	
	//output.color = Ambient;
	//output.color = float4(1.0, 0.0, 0.0, 1.0);
	output.color = tex2D(DiffuseTexSampler, input.tex) * float4(input.color.rgb, 1.0);
	
    return output;
}	

technique TechniqueHLSL
{
	pass Pass1
    {
		//ColorOp[0] = Modulate4x;
    
        VertexShader = compile vs_1_1 VS_AmbientTextured();
        PixelShader = compile ps_1_1 PS_AmbientTextured();
    }
}
