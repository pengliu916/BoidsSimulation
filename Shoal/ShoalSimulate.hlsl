#define blocksize 128

// Shader static constant
static float softeningSquared = 0.0012500000*0.0012500000;
static float softening = 0.0012500000;
//----------------------------------------------
// static constant only for shader development
//----------------------------------------------


// Define how data structured in buffer
struct PosVel{
	float3 pos;
	float3 vel;
};
// Define the temporary intermediate data structure
struct AccPVFN{
	float3 accPos;
	float3 accVel;
	float3 accForce;
	uint accCount;
};
StructuredBuffer<PosVel> oldVP : register(t0);		// Shader resource for input
RWStructuredBuffer<PosVel> newPosVel : register(u0);	// UAV for output
RWStructuredBuffer<AccPVFN> accPVFN : register(u1);
// To improve I/O operations load data into group shared memory
groupshared PosVel sharedFishVP[blocksize];

// Variables in this cbuffer is updated when they have been changed
cbuffer cbCS_perCall : register(c0){	// Variables in this cbuffer is updated when called
	float	visionDistThreshold;		// Meters
	float	visionAngleThreshold;		// cos of angle between vel and dist dir [-1,1]
	float	avoidanceFactor;			// Strength of avoidance force
	float	seperationFactor;			// Strength of seperation force

	float	cohesionFactor;				// Strength of cohesion force
	float	alignmentFactor;			// Strength of alignment force
	float	seekingFactor;				// Strength of seeking force
	float	maximumForce;

	float	maximumSpeed;
	float	minimumSpeed;
	float	fDeltaT;					// Time interval between each simulation step
	uint	uNumInstance;
}

// Calculate the force to avoid other fish
float3 Avoidance(float3 localPos, float3 velDir, float3 avoidPos, float distSqr)
{
	float3 OP = avoidPos - localPos;
	float t = dot(OP, velDir);
	float3 tPos = localPos + velDir * t;
	float3 force = tPos - avoidPos;
	float forceLenSqr = dot(force, force) + softeningSquared;
	return avoidanceFactor * force / (forceLenSqr * distSqr);
}

// Calculate seperation force
float3 Seperation(float3 neighborDir, float3 neighborVel, float invDist)
{
	float3 neighborVelSqr = dot(neighborVel, neighborVel) + softeningSquared;
	float3 invNeighborVelLen = 1.0f / sqrt(neighborVelSqr);
	float3 neighborVelDir = neighborVel * invNeighborVelLen;
	float directionFactor = abs(dot(neighborDir,neighborVelDir))+softening;
	return -seperationFactor * neighborDir * invDist * invDist * ( 1 + 3*directionFactor);
}

// Calculate cohesion force
float3 Cohesion(float3 localPos, float3 avgPos)
{
	float3 delta = avgPos - localPos;
	float deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	return cohesionFactor * delta * invDelta;
}

// Calculate alignment force
float3 Alignment(float3 localVel, float3 avgVel)
{
	float3 delta = avgVel - localVel;
	float3 deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	return alignmentFactor * delta * invDelta;
}

// Calculate seeking force
float3 Seeking(float3 localPos, float3 localVel, float3 seekPos)
{
	float3 delta = seekPos - localPos;
	float deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	float desiredVel = delta * invDelta * maximumSpeed - localVel;
	return seekingFactor * delta * invDelta;
}

[numthreads(blocksize, 1, 1)]
void CS(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	// Each thread of the CS updates one fish
	PosVel localVP = oldVP[DTid.x];

	// Local variable to store temporary values
	float3	accForce	= 0;			// Keep track of all forces for this fish
	float3	accPos		= 0;			// Accumulate neighbor fish pos for neighbor ave pos calculation
	float3	accVel		= 0;			// Accumulate neighbor fish vel for neighbor ave vel calculation
	uint	accCount	= 0;			// Number of near by fish (neighbor) for ave data calculation

	// Update current fish using all other fish
	[loop]
	for (uint tile = 0; tile <= uint(uNumInstance/blocksize); tile++)
	{
		// Cache a tile of particles onto shared memory to increase IO efficiency
		uint currentTileStart = tile * blocksize;
		sharedFishVP[GI] = oldVP[tile * blocksize + GI];
		// Set a group barrier to make sure the entile shared memory is loaded with data
		GroupMemoryBarrierWithGroupSync();

		//[unroll]
		for (uint counter = 0; counter < blocksize; counter++)
		{
			// Calculate distance
			float3 vPos = sharedFishVP[counter].pos - localVP.pos;
			float distSqr = dot(vPos, vPos) + softeningSquared;
			float dist = sqrt(distSqr);
			float invDist = 1.0f / dist;
			// Calculate angle between vel and dist dir
			float3 neighborDir = vPos * invDist;
			float scalarVel = sqrt(dot(localVP.vel, localVP.vel));
			float3 velDir = localVP.vel / scalarVel;
			float cosAngle = dot(velDir, neighborDir);
			// Doing one to one interaction based on visibility
			if(dist <= visionDistThreshold && cosAngle >= visionAngleThreshold)
			{
				accPos += sharedFishVP[counter].pos;	// accumulate neighbor fish pos
				accVel += sharedFishVP[counter].vel;	// accumulate neighbor fish vel
				accCount += 1;							// counting neighbors for calculating avg PV
				// Add seperation and avoidance force
				accForce += Seperation(neighborDir,sharedFishVP[counter].vel, invDist);
				accForce += Avoidance(localVP.pos, velDir, sharedFishVP[counter].pos, distSqr);
			}
		}
		GroupMemoryBarrierWithGroupSync();
	}
	// Calculate average pos and vel of neighbor fish
	if(accCount!=0){
		float3 avgPos = accPos / accCount;
		float3 avgVel = accVel / accCount;
		// Add cohesion alignment forces
		accForce += Cohesion(localVP.pos, avgPos);
		accForce += Alignment(localVP.vel, avgVel);
	}
	accForce += Seeking(localVP.pos, localVP.vel, float3(0,0,0));
	//accForce.y*=0.8;
	float accForceSqr = dot(accForce, accForce);
	if(accForceSqr > maximumForce * maximumForce)
	{
		float invForceLen = 1.0f / sqrt(accForceSqr);
		accForce = accForce * invForceLen * maximumForce;
	}

	localVP.vel += accForce * fDeltaT;      //deltaTime;
	float velAfterSqr = dot(localVP.vel, localVP.vel);
	float invVelLen = 1.0f / sqrt(velAfterSqr);
	if (velAfterSqr > maximumSpeed * maximumSpeed)
	{
		localVP.vel = localVP.vel * invVelLen * maximumSpeed;
	}else if(velAfterSqr < minimumSpeed * minimumSpeed)
	{
		localVP.vel = localVP.vel * invVelLen * minimumSpeed;
	}
	localVP.pos.xyz += localVP.vel.xyz * fDeltaT;        //deltaTime;    

	newPosVel[DTid.x].pos = localVP.pos;
	newPosVel[DTid.x].vel = localVP.vel;

}
