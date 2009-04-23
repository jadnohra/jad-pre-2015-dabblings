#include "Common.fxh"

float4x4 WvpXf;
//float4 Ambient;

struct VSI_Ambient
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

struct VSO_Ambient
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

struct PSO_Ambient
{
    float4 color : COLOR0;
};


VSO_Ambient VS_Ambient(VSI_Ambient input)
{
	VSO_Ambient output;
	
	output.position = mul(input.position, WvpXf);
	output.color = input.color;

    return output;
}	

PSO_Ambient PS_Ambient(VSO_Ambient input)
{
	PSO_Ambient output;
	
	//output.color = Ambient;
	//output.color = float4(1.0, 0.0, 0.0, 1.0);
	output.color = float4(input.color.rgb, 1.0);
	
    return output;
}	

technique TechniqueHLSL
{
	pass Pass1
    {
        VertexShader = compile vs_1_1 VS_Ambient();
        PixelShader = compile ps_1_1 PS_Ambient();
    }
}
