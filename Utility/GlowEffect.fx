Texture2D<float4>    inputTex  : register(t0);
Texture2D<float4>    H_glowTex  : register(t1);
Texture2D<float4>    HV_glowTex  : register(t2);

SamplerState samGeneral : register( s0 );

#define NUMWT 9
float Gauss[NUMWT] = {0.93, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
#define WT_NORMALIZE (1.0/(1.0+2.0*(0.93 + 0.8 + 0.7 + 0.6 + 0.5 + 0.4 + 0.3 + 0.2 + 0.1)))
//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
cbuffer cbPerResize : register( b0 )
{
	int DepthWidth;
	int DepthHeight;
	float glow_factor;
	float blend_factor;
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
float4 PS_Glow_V(PS_INPUT input) : SV_Target
{
		float2 currentLocation = float2( input.Tex.xy ) / float2( DepthWidth, DepthHeight );

	//float4 color = float4( 0, 0, 0, 0 );
	//for( int i = -1*(int)radius; i <= (int)radius; i++ ){
	//	float4 col = inputTex.Load( currentLocation + int3( 0, i, 0 )) / (2.0f*(int)radius +1);
	//	color += col;
	//}
	//color.a=1;
	//return color;
	////return float4 (1,1,1,1);

	float4 c2;
	float4 c = inputTex.Sample( samGeneral, currentLocation ) * (WT_NORMALIZE);
	float2 step = float2( 0, 1.0f / DepthHeight);
	float2 dir = step;
	for(int i=0; i<NUMWT; i++)
	{
		c2 = inputTex.Sample( samGeneral, currentLocation + dir);
		c += c2 * (Gauss[i]*WT_NORMALIZE);
		c2 = inputTex.Sample( samGeneral, currentLocation - dir);
		c += c2 * (Gauss[i]*WT_NORMALIZE);
		dir += step;
	}
	return c * glow_factor;
}

float4 PS_Glow_H(PS_INPUT input) : SV_Target
{
	float2 currentLocation = float2( input.Tex.xy ) / float2( DepthWidth, DepthHeight );
	/*float4 color = float4( 0, 0, 0, 0 );
	for( int i = -1*(int)radius; i <= (int)radius; i++ ){
		float4 col = inputTex.Load( currentLocation + int3( i, 0, 0 )) / (2.0f*(int)radius +1);
		color += col;
	}
	color.a=1;
	return color;*/


	float4 c2;
	float4 c = inputTex.Sample( samGeneral, currentLocation ) * (WT_NORMALIZE);
	float2 step = float2( 1.0f / DepthWidth, 0 );
	float2 dir = step;
	for(int i=0; i<NUMWT; i++)
	{
		c2 = inputTex.Sample( samGeneral, currentLocation + dir);
		c += c2 * (Gauss[i]*WT_NORMALIZE);
		c2 = inputTex.Sample( samGeneral, currentLocation - dir);
		c += c2 * (Gauss[i]*WT_NORMALIZE);
		dir += step;
	}
	return c * glow_factor;
}

float4 PS_Glow_ALL(PS_INPUT input) : SV_Target
{
	float4 color = inputTex.Sample( samGeneral, input.Tex / float2( DepthWidth, DepthHeight ))  +
		HV_glowTex.Sample( samGeneral, input.Tex / float2( DepthWidth , DepthHeight )) *blend_factor ;
	color.a=1;
	return color;
}