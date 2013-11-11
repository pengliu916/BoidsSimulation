Texture2D<float4>    inputTex  : register(t0);
Texture2D<float4>    sumTex  : register(t1);
Texture2D<float4>    tempTex  : register(t2);

SamplerState samGeneral : register( s0 );

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
cbuffer cbPerResize : register( b0 )
{
	int DepthWidth;
	int DepthHeight;
	float blur_factor;
	float radius;
}


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct GS_INPUT
{
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader for every filter
//--------------------------------------------------------------------------------------
GS_INPUT VS( )
{
	GS_INPUT output = (GS_INPUT)0;
 
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader for every filter
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS(point GS_INPUT particles[1], inout TriangleStream<PS_INPUT> triStream)
{
   PS_INPUT output;
	output.Pos=float4(-1.0f,1.0f,0.0f,1.0f);
	output.Tex=float2(0.0f,0.0f);
	triStream.Append(output);

	output.Pos=float4(1.0f,1.0f,0.0f,1.0f);
	output.Tex=float2(DepthWidth,0.0f);
	triStream.Append(output);

	output.Pos=float4(-1.0f,-1.0f,0.0f,1.0f);
	output.Tex=float2(0.0f,DepthHeight);
	triStream.Append(output);

	output.Pos=float4(1.0f,-1.0f,0.0f,1.0f);
	output.Tex=float2(DepthWidth,DepthHeight);
	triStream.Append(output);
}

//--------------------------------------------------------------------------------------
// Pixel Shader just half the distance (test purpose)
//--------------------------------------------------------------------------------------

float4 PS_Combine(PS_INPUT input) : SV_Target
{
	float4 col_input = inputTex.Sample( samGeneral, input.Tex / float2( DepthWidth, DepthHeight ));
	float4 col_sum = sumTex.Sample( samGeneral, input.Tex / float2( DepthWidth , DepthHeight ));
	float4 color = lerp( col_input, col_sum, blur_factor );
	color.a=1;
	return color;
}

float4 PS_Copyback(PS_INPUT input) : SV_Target
{
	float4 color = tempTex.Sample( samGeneral, input.Tex / float2( DepthWidth, DepthHeight ));
	color.a=1;
	return color;
}

float4 PS_Out(PS_INPUT input) : SV_Target
{
	float4 color = sumTex.Sample( samGeneral, input.Tex / float2( DepthWidth, DepthHeight ))+
		inputTex.Sample( samGeneral, input.Tex / float2( DepthWidth, DepthHeight ));
	color.a=1;
	return color;
}