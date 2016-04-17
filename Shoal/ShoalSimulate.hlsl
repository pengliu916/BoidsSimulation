#define blocksize 256
#define PREDATOR 0
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
cbuffer cbCS_perCall : register(b0){	// Variables in this cbuffer is updated when called
	float	avoidanceFactor;			// Strength of avoidance force
	float	seperationFactor;			// Strength of seperation force
	float	cohesionFactor;				// Strength of cohesion force
	float	alignmentFactor;			// Strength of alignment force
	
	float	seekingFactor;				// Strength of seeking force
	float3  seekSourcePos;

	float	fleeFactor;					// Strength of seeking force
	float3  fleeSourcePos;		

	float	maximumForce;
	float3	centerPos;					// Center of the bounding box

	float	maximumSpeed;
	float3	xyzExpand;					// Distance of the maximum corner to centerPos

	float	minimumSpeed;
	float	visionDistThreshold;		// Meters
	float	visionAngleThreshold;		// cos of angle between vel and dist dir [-1,1]
	float	fDeltaT;					// Time interval between each simulation step

	uint	uNumInstance;				
}

static float bondaryFactor = 10.0f;
// Calculate the force from border
float3 BoundaryCorrection( float3 localPos, float3 localVel )
{
	float3 probePos = localPos + localVel * 0.5;
	return probePos;
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
	float deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	return alignmentFactor * delta * invDelta;
}

// Calculate seeking force
float3 Seekings(float3 localPos, float3 localVel, float3 seekPos)
{
	float3 delta = seekPos - localPos;
	float deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	float3 desiredVel = delta * invDelta * maximumSpeed - localVel;
	return seekingFactor * delta *invDelta;
}
float3 Seeking(float3 vLocalPos, float3 vLocalVel, float3 vSeekPos)
{
	float3 vDelta = normalize(vSeekPos - vLocalPos);
		float3 vDesired = vDelta * maximumSpeed;
		return seekingFactor*(vDesired - vLocalVel);
}
// Calculate flee force
float3 Flee(float3 localPos, float3 localVel, float3 fleePos)
{
	float3 delta = localPos - fleePos;
	float deltaSqr = dot(delta, delta) + softeningSquared;
	float invDelta = 1.0f / sqrt(deltaSqr);
	float3 desiredVel = delta * maximumSpeed;
	return fleeFactor*(desiredVel - localVel) *invDelta*invDelta;
}
// Calculate border force from planes
void PlaneVelCorrection( inout float3 probePos )
{
	float3 diff = max( float3( 0, 0, 0 ), probePos - xyzExpand );
	probePos = probePos - diff;
	return;
}
// Calculate border force from edges
void EdgeVelCorrection( float2 probeToEdge, float cornerRadius, inout float2 probePos )
{
	if( all( probeToEdge < int2( 0, 0 ) ) ){
		float dist = length( probeToEdge);
		if( dist > cornerRadius){
			float2 moveDir = normalize( probeToEdge );
			probePos += moveDir * ( dist - cornerRadius );
		}
	}
	return;
}
// Calculate border force from corners
void CornerVelCorrection( float3 probeToCorner, float cornerRadius, inout float3 probPos )
{
	if( all( probeToCorner < int3( 0, 0, 0 ) ) ){
		float dist = length( probeToCorner );
		if( dist > cornerRadius ){
			float3 moveDir = normalize( probeToCorner );
			probPos += moveDir * ( dist - cornerRadius ); 
		}
	}
	return;
}

void BorderVelCorrection( float3 pos, inout float3 vel )
{
	float speed = length( vel );
	float probeDist = speed * 25 * fDeltaT;
	float3 probePos = pos + 25 * fDeltaT * vel;
	float cornerRadius = probeDist * 1.5f;
	int3 posState = pos > float3( 0, 0, 0 );
	int3 convert = posState * 2 - int3( 1, 1, 1 );
	float3 mirrorProbePos = probePos * convert;
	float3 cornerSphereCenterPos = xyzExpand - cornerRadius;
	float3 probeToCorner = cornerSphereCenterPos - mirrorProbePos;
	// For corners
	CornerVelCorrection( probeToCorner, cornerRadius, mirrorProbePos );
	// For edges
	EdgeVelCorrection( probeToCorner.xy, cornerRadius, mirrorProbePos.xy);
	EdgeVelCorrection( probeToCorner.xz, cornerRadius, mirrorProbePos.xz);
	EdgeVelCorrection( probeToCorner.yz, cornerRadius, mirrorProbePos.yz);
	// For planes
	PlaneVelCorrection( mirrorProbePos );
	// Get true new probe pos
	probePos = mirrorProbePos * convert;
	// Get new vel
	vel = speed * normalize( probePos - pos );
}

[numthreads(blocksize, 1, 1)]
void CS(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	// Each thread of the CS updates one fish
	PosVel localVP = oldVP[DTid.x];
	localVP.pos -= centerPos;			// Transform to local space
	float predatorFactor = 1;
	float vDT = visionDistThreshold;
	float vAT = visionAngleThreshold;
#if PREDATOR
	float3 fleePos0 = oldVP[0].pos;
	float3 fleePos1 = oldVP[1].pos;
	if(DTid.x<=1){
		predatorFactor = 0;
		vDT = 10+3*visionDistThreshold;
		vAT = 0.7;
	}
#endif
	// Local variable to store temporary values
	float3	accForce	= 0;			// Keep track of all forces for this fish
	float3	accPos		= 0;			// Accumulate neighbor fish pos for neighbor ave pos calculation
	float3	accVel		= 0;			// Accumulate neighbor fish vel for neighbor ave vel calculation
	uint	accCount	= 0;			// Number of near by fish (neighbor) for ave data calculation

	float scalarVel = sqrt(dot(localVP.vel, localVP.vel));
	float3 velDir = localVP.vel / scalarVel;
	// Update current fish using all other fish
	[loop]
	for (uint tile = 0; tile <= uint(uNumInstance/blocksize); tile++)
	{
		// Cache a tile of particles onto shared memory to increase IO efficiency
		uint currentTileStart = tile * blocksize;
		sharedFishVP[GI] = oldVP[tile * blocksize + GI];
		sharedFishVP[GI].pos -= centerPos;	// Transform to local space
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
			
			float cosAngle = dot(velDir, neighborDir);
			// Doing one to one interaction based on visibility
			if(dist <= vDT && cosAngle >= vAT)
			{
				accPos += sharedFishVP[counter].pos;	// accumulate neighbor fish pos
				accVel += sharedFishVP[counter].vel;	// accumulate neighbor fish vel
				accCount += 1;							// counting neighbors for calculating avg PV
				// Add seperation and avoidance force
				accForce += Seperation(neighborDir,sharedFishVP[counter].vel, invDist)*predatorFactor;
				accForce += Avoidance(localVP.pos, velDir, sharedFishVP[counter].pos, distSqr)*predatorFactor;
#if PREDATOR
				if(DTid.x<=1 && tile+counter<=1 && tile+counter!=DTid.x){
					accForce += Avoidance(localVP.pos, velDir, sharedFishVP[counter].pos, distSqr)*30;
					accForce += Seperation(neighborDir, sharedFishVP[counter].vel, invDist)*30;
				}
#endif
			}
		}
		GroupMemoryBarrierWithGroupSync();
	}
	// Calculate average pos and vel of neighbor fish
	float3 avgPos = float3( 0, 0, 0 );
	if(accCount!=0){
		avgPos = accPos / accCount;
		float3 avgVel = accVel / accCount;
		float3 localFleeSourcePos = fleeSourcePos - centerPos; // Convert flee source pos to local space
		// Add cohesion alignment forces
		accForce += Cohesion(localVP.pos, avgPos + normalize(localVP.vel)*0.2f);
		accForce += Alignment(localVP.vel, avgVel)*predatorFactor;
		accForce += Flee(localVP.pos, localVP.vel, localFleeSourcePos)*predatorFactor;
#if PREDATOR
		accForce += Flee(localVP.pos, localVP.vel, fleePos0)*predatorFactor;
		accForce += Flee(localVP.pos, localVP.vel, fleePos1)*predatorFactor;
#endif
	}
#if PREDATOR
	if(DTid.x<=1 && accCount < 3){
		avgPos = float3(0,0,0);
	}
#endif
	float maxForce;
	float maxSpeed;
	float minSpeed;
	float3 seekPos;
#if PREDATOR
	if (DTid.x<=1) 
	{
		seekPos = avgPos;
		maxForce = 1*maximumForce;
		maxSpeed = 0.7 * maximumSpeed;
		minSpeed = 0; 
	}
	else
#endif
	{
		seekPos = seekSourcePos - centerPos;	// Convert seek source pos to local space
		maxForce = maximumForce;
		maxSpeed = maximumSpeed;
		minSpeed = minimumSpeed;
	}
	float3 seek = Seeking(localVP.pos, localVP.vel, seekPos);
	accForce += seek;
	//accForce.y*=0.8;
	float accForceSqr = dot(accForce, accForce) + softeningSquared;
	float invForceLen = 1.0f / sqrt(accForceSqr);
	float3 forceDir = accForce * invForceLen;
	if (accForceSqr > maxForce * maxForce)
	{
		accForce = forceDir*maximumForce;
	}
#if PREDATOR
	//if(DTid.x<=1) accForce *= (0.5+0.5*clamp(dot(forceDir,velDir),0,1));
#endif
	localVP.vel += accForce * fDeltaT;      //deltaTime;
	float velAfterSqr = dot(localVP.vel, localVP.vel);
	float invVelLen = 1.0f / sqrt(velAfterSqr);
	if (velAfterSqr > maxSpeed * maxSpeed)
	{
		localVP.vel = localVP.vel * invVelLen * maxSpeed;
	} else if (velAfterSqr < minSpeed * minSpeed)
	{
		localVP.vel = localVP.vel * invVelLen * minSpeed;
	}
	BorderVelCorrection( localVP.pos, localVP.vel );
	//localVP.vel = BoundaryCorrection( localVP.pos, localVP.vel );
	localVP.pos.xyz += localVP.vel.xyz * fDeltaT;        //deltaTime;    

	newPosVel[DTid.x].pos = localVP.pos + centerPos;	// Convert the result pos back to world space
	newPosVel[DTid.x].vel = localVP.vel;

}
