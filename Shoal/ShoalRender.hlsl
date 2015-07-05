#define PREDATOR 0
cbuffer cbDraw: register(b0)
{
	float4x4	g_mWorldViewProj;
	float		g_fFishSize;
	float		g_minSpeed;			// For velocity color mapping
	float		g_maxSpeed;			// For velocity color mapping
};

struct PosVel
{
	float3 pos;
	float3 vel;
};

StructuredBuffer<PosVel> g_bufPosVel : register(t0);	// Contain fish vel&pos
Texture2D txFish : register(t1);		// Texture for fish
Texture2D txColor : register(t2);		// Velocity color mapping texture(index table)
SamplerState samGeneral : register(s0);

//static const float3 vLight = float3(0.4242641,0.5656854,0.7071068)£»
struct VS_Input
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
	uint	instanceID : SV_INSTANCEID;	// Used for Shark identification(First 2 instance)
};

struct GS_Input
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
	uint	instanceID : SV_INSTANCEID;
};

struct PS_Input
{
	float4	Pos		: SV_POSITION;
	float2	Tex		: TEXCOORD;
	float	Illu	: ILLUMI;			// Lighting done in GS
	float	Vidx	: VINDEX;			// Index for velocity color mapping
};

GS_Input VS(GS_Input In)
{
	GS_Input output = In;
	return output;
}

[maxvertexcount(3)]
void GS(triangle GS_Input input[3], inout TriangleStream<PS_Input> SpriteStream)
{
	PS_Input output;
	float3 vertex[3];
	float3 pos = g_bufPosVel[input[0].instanceID].pos;
		float3 vel = g_bufPosVel[input[0].instanceID].vel;

	/*	int3 state = pos > float3( 0, 0, 0 );
		state = state * 2 - int3( 1, 1, 1 );
	pos *= state;
	vel *= state;*/

	float velLen = length(vel);

	vel /= velLen;										// Forward vector for each fish
	float3 fup = float3(0, 1, 0);						// Faked up vector for each fish
	float3 right = cross(fup, vel);						// Right vector for each fish
	float3 up = cross(right, vel);
	float3x3 rotMatrix = { vel, up, right };			// Rotation matrix to pose each fish heading forward

	float size = g_fFishSize;							// Set the fish size
	
	// Calculate color index for later velocity color mapping
	float vidx = (velLen - g_minSpeed) / (g_maxSpeed - g_minSpeed);
#if PREDATOR
	if(input[0].instanceID <= 1){
		size*=5;
		vidx = 0.5;
	}
#endif
	[unroll]for (int i = 0; i<3; i++)
	{	// Pose each fish and scale it based on its size
		vertex[i] = mul(input[i].pos*float3(1.5,1,1)*size, rotMatrix);
	}
	// Calculate the normals
	float3 v0 = vertex[1] - vertex[0];
	float3 v1 = vertex[2] - vertex[0];
	float3 n = normalize(cross(v0,v1));
	// Calculate lighting
	float3 vLight = float3(0.4242641, 0.5656854, 0.7071068);
	float illumination = abs(dot(n, vLight));

	[unroll]for (int i = 0; i<3; i++)
	{
		output.Pos = mul(float4(vertex[i]+pos, 1), g_mWorldViewProj);
		output.Tex = input[i].tex;
		output.Vidx = vidx;
		output.Illu = illumination;

		SpriteStream.Append(output);
	}
	SpriteStream.RestartStrip();
}

float4 getColor(Texture2D tex,float2 coord){
	return tex.Sample(samGeneral, coord);
}

float4 PS(PS_Input In) : SV_Target
{
	//float4 col = txColor.Sample(samGeneral, float2(0.48,In.Vidx));
	float4 col = getColor(txColor, float2(0.48,In.Vidx));
	return  col*In.Illu;
}