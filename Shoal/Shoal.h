#pragma once

#define BLOCK_SIZE 256

#include <D3D11.h>
#include <DirectXMath.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include <iostream>
using namespace std;
using namespace DirectX;

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 640
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 480
#endif

struct CB_Render
{
	XMMATRIX		mWorldViewProj;
	float			fSize;
	float			fMinSpeed;
	float			fMaxSpeed;
};

struct CB_Simulation
{
	float			fAvoidanceFactor;
	float			fSeperationFactor;
	float			fCohesionFactor;
	float			fAlignmentFactor;

	float			fSeekingFactor;
	XMFLOAT3		f3SeekSourcePos;

	float			fFleeFactor;
	XMFLOAT3		f3FleeSourcePos;

	float			fMaxForce;
	XMFLOAT3		f3CenterPos;

	float			fMaxSpeed;
	XMFLOAT3		f3xyzExpand;

	float			fMinSpeed;
	float			fVisionDist;
	float			fVisionAngleCos;
	float			fDeltaT;

	UINT			uNumInstance;
	float			niu[3];
};

// Vertex mesh definition For rendering the 'fish'
// Just a quad
struct CB_Immutable
{
	XMVECTOR vQuadVertexPos[4];
	XMVECTOR vQuadVertexTex[4];
};
// Directional Polygon
struct FishModel_Point
{
	float m_vPosTex[5];
};
const FishModel_Point g_PolygonFish[10] = {
	{ -0.3f, 0.0f, -0.3f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ -0.3f, 0.0f, -0.3f, 0.25f, 0.25f },
	{ -1.0f, 1.0f, 0.0f, 0.5f, 0.0f},
	{ -0.3f, 0.0f, 0.3f, 0.75f, 0.25 },
	{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
	{ -0.3f, 0.0f, 0.3f, 0.75f, 0.25f },
	{ -1.0f, -1.0f, 0.0f, 1.0f, 0.5f },
	{ -0.3f, 0.0f, -0.3f, 0.75f, 0.75f },
	{ 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },
};

struct Fish
{
	XMFLOAT3 pos;
	XMFLOAT3 vel;
};

struct AccVPFN		// Struct for accumulated PVF for avg calculation
{
	XMFLOAT3 accPos;
	XMFLOAT3 accVel;
	XMFLOAT3 accForce;
	UINT accCount;
};

struct ShoalData
{
	UINT	uNInstances;
	Fish*	shoal;
};
class Shoal
{
public:
	// D3D objects for render fish
	ID3D11VertexShader*				m_pRenderFishVS;
	ID3D11GeometryShader*			m_pRenderFishGS;
	ID3D11PixelShader*				m_pRenderFishPS;
	ID3D11InputLayout*				m_pRenderFishIL;
	ID3D11Buffer*					m_pRenderFishVB;
	ID3D11Buffer*					m_pRenderCBImmutable;
	ID3D11Buffer*					m_pRenderCBDraw;
	ID3D11RasterizerState*			m_pRenderRS;
	ID3D11ShaderResourceView*		m_pRenderColorTex;
	ID3D11ShaderResourceView*		m_pRenderFishTex;
	ID3D11SamplerState*				m_pRenderFishSS;

	// Compute Shader related objects for crowed simulation
	ID3D11ComputeShader*			m_pSimCS;
	ID3D11Buffer*					m_pSimCB;
	CB_Simulation					m_CBsimParams;
	// Resource for storing and accessing fish's vel pos 
	// for previous frame and new frame
	ID3D11Buffer*					m_pSimFishBuffer[2];
	ID3D11ShaderResourceView*		m_pSimFishBufferSRV[2];
	ID3D11UnorderedAccessView*		m_pSimFishBufferUAV[2];

	// Simulation specs
	UINT							m_uReadBuffer;
	float							m_fFishSize;
	float							m_fInitClusterScale;
	ShoalData						m_sShoalData;

	// Output Image is rendered to offscreen surface
	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pOutputTextureRV;
	ID3D11Texture2D*				m_pOutputStencilTexture2D;
	ID3D11DepthStencilView*			m_pOutputStencilView;

	// Viewport for render to target, along with output image dimension
	D3D11_VIEWPORT					m_RTviewport;
	UINT							m_uRTwidth;
	UINT							m_uRTheight;

	// Use DXUT built-in Model Camera class for mouse interaction
	CModelViewerCamera				m_Camera;

	// Indicate whether using backbuffer for rendering or not
	bool							m_bDirectToBackBuffer;

	// Indicate whether the simulation params are changed or not
	bool							m_bParamChanged;


	Shoal( UINT FishNum, float deltaT = 0.02, XMFLOAT3 centerPos = XMFLOAT3( 0, 0, 0 ),
		XMFLOAT3 xyzExpand = XMFLOAT3( 60, 30, 60 ), bool bRenderToBackbuffer = false,
		UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT )
	{
		m_bDirectToBackBuffer = bRenderToBackbuffer;

		m_uRTheight = height;
		m_uRTwidth = width;

		m_sShoalData.uNInstances = ceil( FishNum / BLOCK_SIZE ) * BLOCK_SIZE;
		m_fFishSize = 0.3f;									// Fish bonding sphere radius in meter
		m_uReadBuffer = 0;

		m_CBsimParams.fAvoidanceFactor = 8.0;	// Strength of avoidance force
		m_CBsimParams.fSeperationFactor = 0.4;	// Strength of seperation force
		m_CBsimParams.fCohesionFactor = 15;		// Strength of cohesion force
		m_CBsimParams.fAlignmentFactor = 12;	// Strength of alignment force
		m_CBsimParams.fSeekingFactor = 0.2;		// Strength of seeking force
		m_CBsimParams.f3SeekSourcePos = centerPos;
		m_CBsimParams.fFleeFactor = 0;			// Strength of flee force
		m_CBsimParams.f3FleeSourcePos = centerPos;
		m_CBsimParams.fMaxForce = 200.0f;		// Maximum power a fish could offer
		m_CBsimParams.f3CenterPos = centerPos;
		m_CBsimParams.fMaxSpeed = 20.0f;		// m/s maximum speed a fish could run
		m_CBsimParams.f3xyzExpand = xyzExpand;
		m_CBsimParams.fMinSpeed = 2.5;			// m/s minimum speed a fish have to maintain
		m_CBsimParams.fVisionDist = 3.5;		// Neighbor dist threshold
		m_CBsimParams.fVisionAngleCos = -0.6;	// Neighbor angle(cos) threshold
		m_CBsimParams.fDeltaT = deltaT;			// seconds of simulation interval
		m_CBsimParams.uNumInstance = m_sShoalData.uNInstances;
		m_bParamChanged = false;
	}

	HRESULT Initial( UINT numFish = -1 )
	{
		HRESULT hr = S_OK;
		if (numFish != -1) m_sShoalData.uNInstances = numFish;
		m_fInitClusterScale = 1.0f;			// Cluster radius in meters
		// Free previous data
		//delete[] m_sShoalData.shoal;

		// Allocate new data
		m_sShoalData.shoal = new Fish[m_sShoalData.uNInstances];
		if (!m_sShoalData.shoal) return E_OUTOFMEMORY;
		float velFactor = 1;
		UINT i = 0;
		while (i < m_sShoalData.uNInstances) {
			float x, y, z;
			x = rand() / (float)RAND_MAX * 2 - 1;
			y = rand() / (float)RAND_MAX * 2 - 1;
			z = rand() / (float)RAND_MAX * 2 - 1;
			x = x*0.2*m_CBsimParams.f3xyzExpand.x + m_CBsimParams.f3CenterPos.x;
			y = y*0.2*m_CBsimParams.f3xyzExpand.y + m_CBsimParams.f3CenterPos.y;
			z = z*0.2*m_CBsimParams.f3xyzExpand.z + m_CBsimParams.f3CenterPos.z;

			XMVECTOR	vPoint = XMLoadFloat3( &XMFLOAT3( x, y, z ) );
			float len;
			XMStoreFloat( &len, XMVector3Length( vPoint ) );
			//if (len > 1)
				//continue;
			//vPoint = XMVector3Normalize(vPoint);
			XMFLOAT3 point;
			XMStoreFloat3( &point, vPoint );

			m_sShoalData.shoal[i].pos.x = point.x * m_fInitClusterScale;
			m_sShoalData.shoal[i].pos.y = point.y * m_fInitClusterScale;
			m_sShoalData.shoal[i].pos.z = point.z * m_fInitClusterScale;
			x = rand() / (float)RAND_MAX * 2 - 1;
			y = rand() / (float)RAND_MAX * 2 - 1;
			z = rand() / (float)RAND_MAX * 2 - 1;
			m_sShoalData.shoal[i].vel.x = x*velFactor;
			m_sShoalData.shoal[i].vel.y = y*velFactor;
			m_sShoalData.shoal[i].vel.z = z*velFactor;

			++i;
		}
		return hr;
	}
	HRESULT CreateShoalPosVelBuffers( ID3D11Device* pd3dDevice )
	{
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = m_sShoalData.uNInstances*sizeof( Fish );
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof( Fish );
		desc.Usage = D3D11_USAGE_DEFAULT;

		// Create the ping-pong position and velocity buffer
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof( InitData ) );
		InitData.pSysMem = m_sShoalData.shoal;
		V( pd3dDevice->CreateBuffer( &desc, &InitData, &m_pSimFishBuffer[0] ) );
		V( pd3dDevice->CreateBuffer( &desc, &InitData, &m_pSimFishBuffer[1] ) );
		DXUT_SetDebugName( m_pSimFishBuffer[0], "m_pSimFishBuffer[0]" );
		DXUT_SetDebugName( m_pSimFishBuffer[1], "m_pSimFishBuffer[1]" );

		// Create shader resource view to give access to rendering shader
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory( &descSRV, sizeof( descSRV ) );
		descSRV.Format = DXGI_FORMAT_UNKNOWN;		// Required for structured buffer
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		descSRV.Buffer.FirstElement = 0;
		descSRV.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
		V( pd3dDevice->CreateShaderResourceView( m_pSimFishBuffer[0], &descSRV, &m_pSimFishBufferSRV[0] ) );
		V( pd3dDevice->CreateShaderResourceView( m_pSimFishBuffer[1], &descSRV, &m_pSimFishBufferSRV[1] ) );
		DXUT_SetDebugName( m_pSimFishBufferSRV[0], "m_pSimFishBufferSRV[0]" );
		DXUT_SetDebugName( m_pSimFishBufferSRV[1], "m_pSimFishBufferSRV[1]" );

		// Create unordered access view for compute shader
		D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
		ZeroMemory( &descUAV, sizeof( descUAV ) );
		descUAV.Format = DXGI_FORMAT_UNKNOWN;
		descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		descUAV.Buffer.FirstElement = 0;
		descUAV.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
		V( pd3dDevice->CreateUnorderedAccessView( m_pSimFishBuffer[0], &descUAV, &m_pSimFishBufferUAV[0] ) );
		V( pd3dDevice->CreateUnorderedAccessView( m_pSimFishBuffer[1], &descUAV, &m_pSimFishBufferUAV[1] ) );
		DXUT_SetDebugName( m_pSimFishBufferUAV[0], "m_pSimFishBufferUAV[0]" );
		DXUT_SetDebugName( m_pSimFishBufferUAV[1], "m_pSimFishBufferUAV[1]" );

		return hr;
	}

	HRESULT CreateResource( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
	{
		HRESULT hr = S_OK;

		// Load shader for rendering
		ID3DBlob* pVSBlob = NULL;
		wstring filename = L"ShoalRender.hlsl";

		V_RETURN( DXUTCompileFromFile( filename.c_str(), nullptr, "VS", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pVSBlob ) );
		V_RETURN( pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pRenderFishVS ) );

		// Create vertex input layout 
		const D3D11_INPUT_ELEMENT_DESC ILdesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		V_RETURN( pd3dDevice->CreateInputLayout( ILdesc, ARRAYSIZE( ILdesc ), pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &m_pRenderFishIL ) );
		DXUT_SetDebugName( m_pRenderFishIL, "m_pRenderFishIL" );
		pVSBlob->Release();


		ID3DBlob* pGSBlob = NULL;
		V_RETURN( DXUTCompileFromFile( filename.c_str(), nullptr, "GS", "gs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pGSBlob ) );
		V_RETURN( pd3dDevice->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pRenderFishGS ) );
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN( DXUTCompileFromFile( filename.c_str(), nullptr, "PS", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pPSBlob ) );
		V_RETURN( pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pRenderFishPS ) );
		pPSBlob->Release();

		// Load compute shader for simulation
		ID3DBlob* pCSBlob = NULL;
		filename = L"ShoalSimulate.hlsl";
		V_RETURN( DXUTCompileFromFile( filename.c_str(), nullptr, "CS", "cs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pCSBlob ) );
		V_RETURN( pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &m_pSimCS ) );
		pCSBlob->Release();

		// Create constant buffer for rendering (for creating quads in render GS)
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		//cbDesc.Usage = D3D11_USAGE_DYNAMIC;			//Dynamic usage uses map and unmap to upload
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.ByteWidth = sizeof( CB_Render );
		V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &m_pRenderCBDraw ) );

		// Create constant buffer for compute shader (for advancing fish)
		cbDesc.ByteWidth = sizeof( CB_Simulation );
		V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &m_pSimCB ) );

		// Create constant buffer for quads generation
		cbDesc.Usage = D3D11_USAGE_IMMUTABLE; // Immutable usage may have better performance?
		cbDesc.CPUAccessFlags = 0;
		cbDesc.ByteWidth = sizeof( CB_Immutable );

		CB_Immutable cbImmutable = {
			XMLoadFloat4( &XMFLOAT4( 0.5f, -0.5f, 0, 0 ) ), XMLoadFloat4( &XMFLOAT4( 0.5f, 0.5f, 0, 0 ) ),
			XMLoadFloat4( &XMFLOAT4( -0.5f, -0.5f, 0, 0 ) ), XMLoadFloat4( &XMFLOAT4( -0.5f, 0.5f, 0, 0 ) ),
			XMLoadFloat4( &XMFLOAT4( 1, 0, 0, 0 ) ), XMLoadFloat4( &XMFLOAT4( 1, 1, 0, 0 ) ),
			XMLoadFloat4( &XMFLOAT4( 0, 0, 0, 0 ) ), XMLoadFloat4( &XMFLOAT4( 0, 1, 0, 0 ) )
		};

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &cbImmutable;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, &initData, &m_pRenderCBImmutable ) );

		// Create compute shader resource
		V( CreateShoalPosVelBuffers( pd3dDevice ) );

		// Create vertex buffer
		D3D11_BUFFER_DESC vbDesc;
		ZeroMemory( &vbDesc, sizeof( D3D11_BUFFER_DESC ) );
		vbDesc.ByteWidth = sizeof( FishModel_Point )* ARRAYSIZE( g_PolygonFish );
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vbInitData;
		ZeroMemory( &vbInitData, sizeof( vbInitData ) );
		vbInitData.pSysMem = g_PolygonFish;
		V_RETURN( pd3dDevice->CreateBuffer( &vbDesc, &vbInitData, &m_pRenderFishVB ) );
		DXUT_SetDebugName( m_pRenderFishVB, "m_pRenderFishVB" );

		// Create texture resource for fish rendering
		V_RETURN( DXUTCreateShaderResourceViewFromFile( pd3dDevice, L"misc\\Fish.png", &m_pRenderFishTex ) );
		V_RETURN( DXUTCreateShaderResourceViewFromFile( pd3dDevice, L"misc\\colorMap.jpg", &m_pRenderColorTex ) );

		// Create sampler for fish texture mapping
		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof( sampDesc ) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN( pd3dDevice->CreateSamplerState( &sampDesc, &m_pRenderFishSS ) );

		// Create rendertarget resource
		if (!m_bDirectToBackBuffer)
		{
			D3D11_TEXTURE2D_DESC	RTtextureDesc = {0};
			RTtextureDesc.Width = m_uRTwidth;
			RTtextureDesc.Height = m_uRTheight;
			RTtextureDesc.MipLevels = 1;
			RTtextureDesc.ArraySize = 1;
			//RTtextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			RTtextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			RTtextureDesc.SampleDesc.Count = 1;
			RTtextureDesc.SampleDesc.Quality = 0;
			RTtextureDesc.Usage = D3D11_USAGE_DEFAULT;
			RTtextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			RTtextureDesc.CPUAccessFlags = 0;
			RTtextureDesc.MiscFlags = 0;
			V_RETURN( pd3dDevice->CreateTexture2D( &RTtextureDesc, NULL, &m_pOutputTexture2D ) );

			D3D11_RENDER_TARGET_VIEW_DESC		RTViewDesc;
			ZeroMemory( &RTViewDesc, sizeof( RTViewDesc ) );
			RTViewDesc.Format = RTtextureDesc.Format;
			RTViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RTViewDesc.Texture2D.MipSlice = 0;
			V_RETURN( pd3dDevice->CreateRenderTargetView( m_pOutputTexture2D, &RTViewDesc, &m_pOutputTextureRTV ) );

			D3D11_SHADER_RESOURCE_VIEW_DESC		SRViewDesc;
			ZeroMemory( &SRViewDesc, sizeof( SRViewDesc ) );
			SRViewDesc.Format = RTtextureDesc.Format;
			SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRViewDesc.Texture2D.MostDetailedMip = 0;
			SRViewDesc.Texture2D.MipLevels = 1;
			V_RETURN( pd3dDevice->CreateShaderResourceView( m_pOutputTexture2D, &SRViewDesc, &m_pOutputTextureRV ) );
		}
		//Create DepthStencil buffer and view
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory( &descDepth, sizeof( descDepth ) );
		descDepth.Width = m_uRTwidth;
		descDepth.Height = m_uRTheight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXUTGetDeviceSettings().d3d11.AutoDepthStencilFormat;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pOutputStencilTexture2D );


		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory( &descDSV, sizeof( descDSV ) );
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		V_RETURN( pd3dDevice->CreateDepthStencilView( m_pOutputStencilTexture2D, // Depth stencil texture
			&descDSV, // Depth stencil desc
			&m_pOutputStencilView ) );  // [out] Depth stencil view

	// rasterizer state
		D3D11_RASTERIZER_DESC rsDesc;
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = FALSE;
		rsDesc.DepthBias = 0;
		rsDesc.DepthBiasClamp = 0.0f;
		rsDesc.SlopeScaledDepthBias = 0.0f;
		rsDesc.DepthClipEnable = TRUE;
		rsDesc.ScissorEnable = FALSE;
		rsDesc.MultisampleEnable = FALSE;
		rsDesc.AntialiasedLineEnable = FALSE;
		V_RETURN( pd3dDevice->CreateRasterizerState( &rsDesc, &m_pRenderRS ) );

		m_RTviewport.Width = (float)m_uRTwidth;
		m_RTviewport.Height = (float)m_uRTheight;
		m_RTviewport.MinDepth = 0.0f;
		m_RTviewport.MaxDepth = 1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		XMVECTORF32 vecEye = {0.0f, 0.0f, -100.0f, 0.f};
		XMVECTORF32 vecAt = {0.0f, 0.0f, 0.0f, 0.f};
		//m_Camera.SetViewParams(vecEye, vecAt);
		m_Camera.SetRadius( 80.0f, 1.0f, 10000.0f );

		return hr;
	}

	void Resize()
	{
		float fAspectRatio = m_uRTwidth / (FLOAT)m_uRTheight;
		m_Camera.SetProjParams( XM_PI / 4, fAspectRatio, 0.1f, 100000.0f );
		m_Camera.SetWindow( m_uRTwidth, m_uRTheight );
		m_Camera.SetButtonMasks();

		// Updating constant buffer
		ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
		m_pOutputTextureRTV = DXUTGetD3D11RenderTargetView();
		pd3dImmediateContext->UpdateSubresource( m_pSimCB, 0, NULL, &m_CBsimParams, 0, 0 );
		/*D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dImmediateContext->Map(m_pSimCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		MappedResource.pData = &m_CBsimParams;
		pd3dImmediateContext->Unmap(m_pSimCB, 0);*/
		//SAFE_RELEASE(pd3dImmediateContext);
	}

	void Update( float fElapsedTime )
	{
		m_Camera.FrameMove( fElapsedTime );
		m_CBsimParams.fDeltaT = fElapsedTime;
		m_bParamChanged = true;
	}

	void Simulate( ID3D11DeviceContext* pd3dImmediateContext )
	{
		int dimx = int( ceil( m_sShoalData.uNInstances / (float)BLOCK_SIZE ) );
		pd3dImmediateContext->CSSetShader( m_pSimCS, NULL, 0 );

		// Set CS input
		pd3dImmediateContext->CSSetShaderResources( 0, 1, &m_pSimFishBufferSRV[1 - m_uReadBuffer] );
		// Set CS output
		UINT initCounts = 0;
		pd3dImmediateContext->CSSetUnorderedAccessViews( 0, 1, &m_pSimFishBufferUAV[m_uReadBuffer], &initCounts );

		if (m_bParamChanged)
		{
			pd3dImmediateContext->UpdateSubresource( m_pSimCB, 0, NULL, &m_CBsimParams, 0, 0 );
			m_bParamChanged = false;
		}
		// Set CS constant buffer
		pd3dImmediateContext->CSSetConstantBuffers( 0, 1, &m_pSimCB );
		// Run the CS
		pd3dImmediateContext->Dispatch( dimx, 1, 1 );
		// Unbind resources for CS
		ID3D11UnorderedAccessView* ppUAViewNULL[1] = {NULL};
		pd3dImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, &initCounts );
		ID3D11ShaderResourceView* ppSRVNULL[1] = {NULL};
		pd3dImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );
		//m_uReadBuffer = 1 - m_uReadBuffer;
	}

	void Render( ID3D11DeviceContext* pd3dImmediateContext )
	{
		HRESULT hr = S_OK;
		//Simulate(pd3dImmediateContext);
		// Clear rendertarget
		float ClearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		pd3dImmediateContext->ClearRenderTargetView( m_pOutputTextureRTV, ClearColor );
		pd3dImmediateContext->ClearDepthStencilView( m_pOutputStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

		XMMATRIX m_Proj = m_Camera.GetProjMatrix();
		//{// Orthographic Projection
		//	XMVECTOR eyePos = m_Camera.GetEyePt();
		//	float length;
		//	XMStoreFloat(&length, XMVector3Length(eyePos));
		//	XMMATRIX m_Proj = XMMatrixOrthographicLH(length, (float)m_uRTheight / (float)m_uRTheight*length, 0.1, 1000);
		//}
		XMMATRIX m_View = m_Camera.GetViewMatrix();
		XMMATRIX m_World = m_Camera.GetWorldMatrix();

		// Updating rendering constant buffer
		CB_Render CBRender;
		CBRender.mWorldViewProj = XMMatrixTranspose( m_World*m_View*m_Proj );
		CBRender.fSize = m_fFishSize;
		CBRender.fMinSpeed = m_CBsimParams.fMinSpeed;
		CBRender.fMaxSpeed = m_CBsimParams.fMaxSpeed;
		pd3dImmediateContext->UpdateSubresource( m_pRenderCBDraw, 0, NULL, &CBRender, 0, 0 );


		// Set shaders
		pd3dImmediateContext->VSSetShader( m_pRenderFishVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pRenderFishGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pRenderFishPS, NULL, 0 );
		// Bind constant buffer
		pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pRenderCBDraw );
		// Bind VS shader resource, the StructuredBuffer
		pd3dImmediateContext->GSSetShaderResources( 0, 1, &m_pSimFishBufferSRV[m_uReadBuffer] );
		pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pRenderFishTex );
		pd3dImmediateContext->PSSetShaderResources( 2, 1, &m_pRenderColorTex );
		pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pRenderFishSS );

		// Set viewport
		pd3dImmediateContext->RSSetViewports( 1, &m_RTviewport );
		// Set rendertarget
		pd3dImmediateContext->OMSetRenderTargets( 1, &m_pOutputTextureRTV, m_pOutputStencilView );
		pd3dImmediateContext->RSSetState( m_pRenderRS );

		// Set the input assembler, and vertex buffer
		pd3dImmediateContext->IASetInputLayout( m_pRenderFishIL );
		UINT Stride = sizeof( FishModel_Point );
		UINT Offset = 0;
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pRenderFishVB, &Stride, &Offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

		// Draw
		pd3dImmediateContext->DrawInstanced( ARRAYSIZE( g_PolygonFish ), m_sShoalData.uNInstances, 0, 0 );

		ID3D11ShaderResourceView* ppSRVNULL[1] = {NULL};
		pd3dImmediateContext->GSSetShaderResources( 0, 1, ppSRVNULL );
	}

	void Release()
	{
		SAFE_RELEASE( m_pRenderFishVS );
		SAFE_RELEASE( m_pRenderFishPS );
		SAFE_RELEASE( m_pRenderFishGS );
		SAFE_RELEASE( m_pRenderFishIL );
		SAFE_RELEASE( m_pRenderFishVB );
		SAFE_RELEASE( m_pRenderColorTex );
		SAFE_RELEASE( m_pRenderFishTex );
		SAFE_RELEASE( m_pRenderFishSS );
		SAFE_RELEASE( m_pRenderCBImmutable );
		SAFE_RELEASE( m_pRenderCBDraw );
		SAFE_RELEASE( m_pRenderRS );

		SAFE_RELEASE( m_pSimCS );
		SAFE_RELEASE( m_pSimCB );
		SAFE_RELEASE( m_pSimFishBuffer[0] );
		SAFE_RELEASE( m_pSimFishBuffer[1] );
		SAFE_RELEASE( m_pSimFishBufferSRV[0] );
		SAFE_RELEASE( m_pSimFishBufferSRV[1] );
		SAFE_RELEASE( m_pSimFishBufferUAV[0] );
		SAFE_RELEASE( m_pSimFishBufferUAV[1] );

		SAFE_RELEASE( m_pOutputStencilTexture2D );
		SAFE_RELEASE( m_pOutputStencilView );

		if (!m_bDirectToBackBuffer)
		{
			SAFE_RELEASE( m_pOutputTexture2D );
			SAFE_RELEASE( m_pOutputTextureRTV );
			SAFE_RELEASE( m_pOutputTextureRV );
		}
	}

	LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		UNREFERENCED_PARAMETER( lParam );
		UNREFERENCED_PARAMETER( hWnd );
		m_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

		return 0;
	}
};