cbuffer cbDraw: register(c0)
{
	float4x4	g_mWorldViewProj;
	float		g_fFishSize;
	float		g_minSpeed;
	float		g_maxSpeed;
};

struct PosVel
{
	float3 pos;
	float3 vel;
};

StructuredBuffer<PosVel> g_bufPosVel : register(t0);
Texture2D txFish : register(t1);
Texture2D txColor : register(t2);
SamplerState samGeneral : register(s0);

struct VS_Input
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
	uint	instanceID : SV_INSTANCEID;
};

struct GS_Input
{
	float4	Pos		: SV_POSITION;
	float2	Tex		: TEXCOORD;
	float4	Col		: COLOR0;
	float4	VCol	: COLOR1;
};

struct PS_Input
{
	float4	Pos		: SV_POSITION;
	float4	Col		: COLOR0;
	float4	VCol	: COLOR1;
	float2	Tex		: TEXCOORD;
};

GS_Input VS(VS_Input In)
{
	GS_Input output = (GS_Input)0;
	float3 pos = g_bufPosVel[In.instanceID].pos;
	float3 vel = g_bufPosVel[In.instanceID].vel;
	float velLen = length(vel);
	vel /= velLen;
	float3 fup = float3(0,1,0);
	float3 right = cross(fup, vel);
	float3 up = cross(right,vel);

	float3x3 rotMatrix = {vel,up,right};
	float3 rotPos = mul(In.pos*g_fFishSize,rotMatrix);
	//float4 pos = float4((In.vID % 100) / 100.f, (In.vID / 100) / 100.f, 0, 1);
	output.Pos = mul(float4(pos+rotPos,1), g_mWorldViewProj);
	output.Tex = In.tex;
	output.Col = float4(abs(normalize(pos.xyz)), 1);
	int3 uv = int3(130, (int)(640.0f*((velLen - g_minSpeed) / (g_maxSpeed - g_minSpeed))),0);
	output.VCol = txColor.Load(uv);
	return output;
}

[maxvertexcount(3)]
void GS(triangle GS_Input input[3], inout TriangleStream<PS_Input> SpriteStream)
{
	PS_Input output;

	// Emit two new triangles
	[unroll]for (int i = 0; i<3; i++)
	{
		output.Pos = input[i].Pos ;
		output.Tex = input[i].Tex;
		output.Col = input[i].Col;
		output.VCol = input[i].VCol;

		SpriteStream.Append(output);
	}
	SpriteStream.RestartStrip();
}

float4 PS(PS_Input In) : SV_Target
{
	float alpha = txFish.Sample(samGeneral, In.Tex).a;
	return  alpha * In.VCol+(1-alpha)*In.Col;
}