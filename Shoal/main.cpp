#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "DXUTgui.h"
#define SUB_TEXTUREWIDTH 1440
#define SUB_TEXTUREHEIGHT 900

#include "MultiTexturePresenter.h"
#include "GlowEffect.h"
#include "MotionBlurEffect.h"
#include "Shoal.h"


//--------------------------------------------------------------------------------------
//Global Variables
//--------------------------------------------------------------------------------------
float								g_fDeltaTime = 0.015;	// Global dt for simulation
float								g_fThreasholdDt = 0.08;	// Global maximum dt allowed for simulation
float								g_fdtAccumulator = 0;	// Global accumulator for dt

//MultiTexturePresenter						MultiTexture = MultiTexturePresenter( 1, true, SUB_TEXTUREWIDTH, SUB_TEXTUREHEIGHT );
CDXUTDialogResourceManager					DialogResourceManager;
CDXUTDialog									SimulationUI;
//CDXUTDialog									RenderUI;

Shoal										FishCluster = Shoal( 20000, g_fDeltaTime,DirectX::XMFLOAT3(0,0,0),DirectX::XMFLOAT3(60,30,60),true,1280,800 );
//GlowEffect									PostEffect_Glow = GlowEffect();
//MotionBlurEffect							PostEffect_Blur = MotionBlurEffect();


bool										g_bRenderUI = true;




static uint64_t g_tickesPerSecond;
static uint64_t g_lastFrameTickCount;

double g_elapsedTime = 0;
double frameTime = 0;
double g_deltaTime = 0;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------

#define IDC_VISIONDIST_STATIC		1
#define IDC_VISIONDIST_SLIDER		2
#define IDC_VISIONANGLE_STATIC		3
#define IDC_VISIONANGLE_SLIDER		4

#define IDC_AVOIDANCE_STATIC		5
#define IDC_AVOIDANCE_SLIDER		6
#define IDC_SEPERATIONFORCE_STATIC	7
#define IDC_SEPERATIONFORCE_SLIDER	8
#define IDC_COHESIONFORCE_STATIC	9
#define IDC_COHESIONFORCE_SLIDER	10
#define IDC_ALIGNMENTFORCE_STATIC	11
#define IDC_ALIGNMENTFORCE_SLIDER	12
#define IDC_SEEKINGFORCE_STATIC		13
#define IDC_SEEKINGFORCE_SLIDER		14
#define IDC_FLEEFACTOR_STATIC		15
#define IDC_FLEEFACTOR_SLIDER		16

#define IDC_MAXFORCE_STATIC			17
#define IDC_MAXFORCE_SLIDER			18
#define IDC_MAXSPEED_STATIC			19
#define IDC_MAXSPEED_SLIDER			20
#define IDC_MINSPEED_STATIC			21
#define IDC_MINSPEED_SLIDER			22

#define IDC_FISHSIZE_STATIC			23
#define IDC_FISHSIZE_SLIDER			24

#define IDC_GLOWFACTOR_STATIC		25
#define IDC_GLOWFACTOR_SLIDER		26
#define IDC_GLOWBLENDFACTOR_STATIC	27
#define IDC_GLOWBLENDFACTOR_SLIDER	28
#define IDC_BLURFACTOR_STATIC		29
#define IDC_BLURFACTOR_SLIDER		30

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

//--------------------------------------------------------------------------------------
//Initialization
//--------------------------------------------------------------------------------------
HRESULT Initial()
{
	HRESULT hr = S_OK;

	SimulationUI.Init( &DialogResourceManager );
	//SimulationUI.SetFont
	SimulationUI.SetCallback( OnGUIEvent ); int iY = 10;

	SimulationUI.SetFont( 1, L"Comic Sans MS", 400, 400 );
	SimulationUI.SetFont( 2, L"Courier New", 16, FW_NORMAL );

	WCHAR sz[100];
	iY += 24;
	swprintf_s( sz, 100, L"VisionDist: %0.2f", FishCluster.m_CBsimParams.fVisionDist );
	SimulationUI.AddStatic( IDC_VISIONDIST_STATIC, sz, 0, iY, 170, 23 );
	SimulationUI.AddSlider( IDC_VISIONDIST_SLIDER, 0, iY += 26, 170, 23, 0, 200,
							( int )FishCluster.m_CBsimParams.fVisionDist * 10 );

	swprintf_s( sz, 100, L"VisionAngleCos: %0.2f", FishCluster.m_CBsimParams.fVisionAngleCos );
	SimulationUI.AddStatic( IDC_VISIONANGLE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_VISIONANGLE_SLIDER, 0, iY += 26, 170, 23, -100, 100,
							( int )( FishCluster.m_CBsimParams.fVisionAngleCos * 100 ) );

	swprintf_s( sz, 100, L"AvoidanceForce: %0.2f", FishCluster.m_CBsimParams.fAvoidanceFactor );
	SimulationUI.AddStatic( IDC_AVOIDANCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_AVOIDANCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
							( int )( FishCluster.m_CBsimParams.fAvoidanceFactor * 10 ) );

	swprintf_s( sz, 100, L"SeperationForce: %0.2f", FishCluster.m_CBsimParams.fSeperationFactor );
	SimulationUI.AddStatic( IDC_SEPERATIONFORCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_SEPERATIONFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
							( int )( FishCluster.m_CBsimParams.fSeperationFactor * 10 ) );

	swprintf_s( sz, 100, L"CohesionForce: %0.2f", FishCluster.m_CBsimParams.fCohesionFactor );
	SimulationUI.AddStatic( IDC_COHESIONFORCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_COHESIONFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 200,
							( int )( FishCluster.m_CBsimParams.fCohesionFactor * 10 ) );

	swprintf_s( sz, 100, L"AlignmentForce: %0.2f", FishCluster.m_CBsimParams.fAlignmentFactor );
	SimulationUI.AddStatic( IDC_ALIGNMENTFORCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_ALIGNMENTFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 200,
							( int )( FishCluster.m_CBsimParams.fAlignmentFactor * 10 ) );

	swprintf_s( sz, 100, L"SeekingForce: %0.2f", FishCluster.m_CBsimParams.fSeekingFactor );
	SimulationUI.AddStatic( IDC_SEEKINGFORCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_SEEKINGFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 500,
							( int )( FishCluster.m_CBsimParams.fSeekingFactor * 100 ) );

	swprintf_s( sz, 100, L"FleeForce: %0.2f", FishCluster.m_CBsimParams.fFleeFactor );
	SimulationUI.AddStatic( IDC_FLEEFACTOR_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_FLEEFACTOR_SLIDER, 0, iY += 26, 170, 23, 10, 1000,
							( int )( FishCluster.m_CBsimParams.fFleeFactor * 10 ) );

	swprintf_s( sz, 100, L"MaxForce: %0.2f", FishCluster.m_CBsimParams.fMaxForce );
	SimulationUI.AddStatic( IDC_MAXFORCE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_MAXFORCE_SLIDER, 0, iY += 26, 170, 23, 10, 5000,
							( int )( FishCluster.m_CBsimParams.fMaxForce * 10 ) );

	swprintf_s( sz, 100, L"MaxSpeed: %0.2f", FishCluster.m_CBsimParams.fMaxSpeed );
	SimulationUI.AddStatic( IDC_MAXSPEED_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_MAXSPEED_SLIDER, 0, iY += 26, 170, 23, 10, 500,
							( int )( FishCluster.m_CBsimParams.fMaxSpeed * 10 ) );

	swprintf_s( sz, 100, L"MinSpeed: %0.2f", FishCluster.m_CBsimParams.fMinSpeed );
	SimulationUI.AddStatic( IDC_MINSPEED_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_MINSPEED_SLIDER, 0, iY += 26, 170, 23, 0, 400,
							( int )( FishCluster.m_CBsimParams.fMinSpeed * 10 ) );

	swprintf_s( sz, 100, L"FishSize: %0.2f", FishCluster.m_fFishSize );
	SimulationUI.AddStatic( IDC_FISHSIZE_STATIC, sz, 0, iY += 26, 170, 23 );
	SimulationUI.AddSlider( IDC_FISHSIZE_SLIDER, 0, iY += 26, 170, 23, 1, 2000,
							( int )( FishCluster.m_fFishSize * 100 ) );

	//RenderUI.Init( &DialogResourceManager );
	////RenderUI.SetFont
	//RenderUI.SetCallback( OnGUIEvent ); iY = 10;

	//RenderUI.SetFont( 1, L"Comic Sans MS", 400, 400 );
	//RenderUI.SetFont( 2, L"Courier New", 16, FW_NORMAL );
	//swprintf_s( sz, 100, L"Glow factor: %0.2f", PostEffect_Glow.m_CBperResize.glow_factor );
	//RenderUI.AddStatic( IDC_GLOWFACTOR_STATIC, sz, 0, iY += 26, 170, 23 );
	//RenderUI.AddSlider( IDC_GLOWFACTOR_SLIDER, 0, iY += 26, 170, 23, 0, 200, 100 );

	//swprintf_s( sz, 100, L"Blend factor: %0.2f", PostEffect_Glow.m_CBperResize.blend_factor );
	//RenderUI.AddStatic( IDC_GLOWBLENDFACTOR_STATIC, sz, 0, iY += 26, 170, 23 );
	//RenderUI.AddSlider( IDC_GLOWBLENDFACTOR_SLIDER, 0, iY += 26, 170, 23, 0, 300, 100 );

	//swprintf_s( sz, 100, L"Blur factor: %0.2f", PostEffect_Blur.m_CBperResize.blur_factor );
	//RenderUI.AddStatic( IDC_BLURFACTOR_STATIC, sz, 0, iY += 26, 170, 23 );
	//RenderUI.AddSlider( IDC_BLURFACTOR_SLIDER, 0, iY += 26, 170, 23, 100, 1000, 600 );

	FishCluster.Initial();
	//V_RETURN( MultiTexture.Initial() );
	//V_RETURN( PostEffect_Glow.Initial() );
	//V_RETURN( PostEffect_Blur.Initial() );
	return hr;
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									   DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	//MultiTexture.ModifyDeviceSettings( pDeviceSettings );
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									  void* pUserContext )
{
	HRESULT hr = S_OK;
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN( DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );

	V_RETURN( FishCluster.CreateResource( pd3dDevice, pd3dImmediateContext ) );
	//V_RETURN( PostEffect_Blur.CreateResource( pd3dDevice, FishCluster.m_pOutputTextureRV ) );
	//V_RETURN( PostEffect_Glow.CreateResource( pd3dDevice, PostEffect_Blur.m_pOutputTextureSRV, &FishCluster.m_Camera ) );
	//V_RETURN( MultiTexture.CreateResource( pd3dDevice, PostEffect_Glow.m_pOutputTextureSRV ) );
	//V_RETURN(MultiTexture.CreateResource(pd3dDevice, PostEffect_Glow.m_pGlow_H_TextureSRV));

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										  const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;
	V_RETURN( DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
	SimulationUI.SetLocation( pBackBufferSurfaceDesc->Width - 180, 0 );
	SimulationUI.SetSize( 180, 600 );

	//RenderUI.SetLocation( 20, 0 );
	//RenderUI.SetSize( 180, 600 );

	//MultiTexture.Resize();
	FishCluster.Resize();

	QueryPerformanceFrequency( (LARGE_INTEGER*)&g_tickesPerSecond );
	QueryPerformanceCounter( (LARGE_INTEGER*)&g_lastFrameTickCount );

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	FishCluster.Update( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								  double fTime, float fElapsedTime, void* pUserContext )
{

	uint64_t count;
	QueryPerformanceCounter( (LARGE_INTEGER*)&count );
	double g_deltaTime = (double)(count - g_lastFrameTickCount) / g_tickesPerSecond;
	g_elapsedTime += g_deltaTime;
	g_lastFrameTickCount = count;

	// Maintaining absolute time sync is not important in this demo so we can err on the "smoother" side
	double alpha = 0.1f;
	frameTime = alpha * g_deltaTime + (1.0f - alpha) * frameTime;

	// Update GUI
	{
		wchar_t buffer[512];
		swprintf( buffer, 512, L"-%4.1f ms  %.0f fps", 1000.f * frameTime, 1.0f / frameTime );
		SetWindowText(DXUTGetHWNDDeviceWindowed(), buffer );
	}

	/*if (fElapsedTime > g_fThreasholdDt) fElapsedTime = g_fThreasholdDt;
	g_fdtAccumulator += fElapsedTime;
	while (g_fdtAccumulator >= g_fDeltaTime){
		g_fdtAccumulator -= g_fDeltaTime;
	}*/
		FishCluster.Simulate(pd3dImmediateContext);
	FishCluster.Render( pd3dImmediateContext );
	//PostEffect_Blur.Render( pd3dImmediateContext );
	//PostEffect_Glow.Render( pd3dImmediateContext );
	//MultiTexture.Render( pd3dImmediateContext );

	//if( g_bRenderUI )
	//{
	//	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR2, L"SimulationUI" );
	//	SimulationUI.OnRender( fElapsedTime );
	//	//RenderUI.OnRender( fElapsedTime );
	//	DXUT_EndPerfEvent();
	//}
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{

	DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	FishCluster.Release();

	//PostEffect_Blur.Release();
	//PostEffect_Glow.Release();
	//MultiTexture.Release();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						  bool* pbNoFurtherProcessing, void* pUserContext )
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
	if( g_bRenderUI ){
		// Give the dialogs a chance to handle the message first
		*pbNoFurtherProcessing = SimulationUI.MsgProc( hWnd, uMsg, wParam, lParam );
		if( *pbNoFurtherProcessing )
			return 0;
		// Give the dialogs a chance to handle the message first
		/**pbNoFurtherProcessing = RenderUI.MsgProc( hWnd, uMsg, wParam, lParam );
		if( *pbNoFurtherProcessing )
			return 0;*/
	}
	FishCluster.HandleMessages( hWnd, uMsg, wParam, lParam );
	switch( uMsg )
	{
	case WM_KEYDOWN:
		int nKey = static_cast< int >( wParam );
		if( nKey == 'H' )
		{
			g_bRenderUI = !g_bRenderUI;
		}
		break;
	}
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					   bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					   int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	WCHAR sz[100];
	float floatNum = 0;
	switch( nControlID )
	{
	case IDC_VISIONDIST_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_VISIONDIST_SLIDER )->GetValue() * 0.1f );
		swprintf_s( sz, 100, L"VisionDist: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fVisionDist = floatNum;
		SimulationUI.GetStatic( IDC_VISIONDIST_STATIC )->SetText( sz );
		break;

	case IDC_VISIONANGLE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_VISIONANGLE_SLIDER )->GetValue() * 0.01f );
		swprintf_s( sz, 100, L"VisionAngleCos: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fVisionAngleCos = floatNum;
		SimulationUI.GetStatic( IDC_VISIONANGLE_STATIC )->SetText( sz );
		break;

	case IDC_AVOIDANCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_AVOIDANCE_SLIDER )->GetValue()*0.1f );
		swprintf_s( sz, 100, L"AvoidanceForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fAvoidanceFactor = floatNum;
		SimulationUI.GetStatic( IDC_AVOIDANCE_STATIC )->SetText( sz );
		break;

	case IDC_SEPERATIONFORCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_SEPERATIONFORCE_SLIDER )->GetValue() * 0.1f );
		swprintf_s( sz, 100, L"SeperationForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fSeperationFactor = floatNum;
		SimulationUI.GetStatic( IDC_SEPERATIONFORCE_STATIC )->SetText( sz );
		break;

	case IDC_COHESIONFORCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_COHESIONFORCE_SLIDER )->GetValue() * 0.1f );
		swprintf_s( sz, 100, L"CohesionForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fCohesionFactor = floatNum;
		SimulationUI.GetStatic( IDC_COHESIONFORCE_STATIC )->SetText( sz );
		break;

	case IDC_ALIGNMENTFORCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_ALIGNMENTFORCE_SLIDER )->GetValue()*0.1f );
		swprintf_s( sz, 100, L"AlignmentForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fAlignmentFactor = floatNum;
		SimulationUI.GetStatic( IDC_ALIGNMENTFORCE_STATIC )->SetText( sz );
		break;

	case IDC_SEEKINGFORCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_SEEKINGFORCE_SLIDER )->GetValue()*0.01f );
		swprintf_s( sz, 100, L"SeekingForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fSeekingFactor = floatNum;
		SimulationUI.GetStatic( IDC_SEEKINGFORCE_STATIC )->SetText( sz );
		break;

	case IDC_FLEEFACTOR_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_FLEEFACTOR_SLIDER )->GetValue()*0.1f );
		swprintf_s( sz, 100, L"FleeForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fFleeFactor = floatNum;
		SimulationUI.GetStatic( IDC_FLEEFACTOR_STATIC )->SetText( sz );
		break;

	case IDC_MAXFORCE_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_MAXFORCE_SLIDER )->GetValue() * 0.1f );
		swprintf_s( sz, 100, L"MaxForce: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fMaxForce = floatNum;
		SimulationUI.GetStatic( IDC_MAXFORCE_STATIC )->SetText( sz );
		break;

	case IDC_MAXSPEED_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_MAXSPEED_SLIDER )->GetValue() *0.1f );
		swprintf_s( sz, 100, L"MaxSpeed: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fMaxSpeed = floatNum;
		SimulationUI.GetStatic( IDC_MAXSPEED_STATIC )->SetText( sz );
		break;

	case IDC_MINSPEED_SLIDER:
		FishCluster.m_bParamChanged = true;
		floatNum = ( float )( SimulationUI.GetSlider( IDC_MINSPEED_SLIDER )->GetValue() *0.1f );
		swprintf_s( sz, 100, L"MinSpeed: %0.2f", floatNum );
		FishCluster.m_CBsimParams.fMinSpeed = floatNum;
		SimulationUI.GetStatic( IDC_MINSPEED_STATIC )->SetText( sz );
		break;

	case IDC_FISHSIZE_SLIDER:
		floatNum = ( float )( SimulationUI.GetSlider( IDC_FISHSIZE_SLIDER )->GetValue() *0.01f );
		swprintf_s( sz, 100, L"FishSize: %0.2f", floatNum );
		FishCluster.m_fFishSize = floatNum;
		SimulationUI.GetStatic( IDC_FISHSIZE_STATIC )->SetText( sz );
		break;

	//case IDC_GLOWFACTOR_SLIDER:
	//	floatNum = ( float )( RenderUI.GetSlider( IDC_GLOWFACTOR_SLIDER )->GetValue() * 0.01f );
	//	swprintf_s( sz, 100, L"Glow Factor: %0.2f", floatNum );
	//	PostEffect_Glow.m_CBperResize.glow_factor = floatNum;
	//	RenderUI.GetStatic( IDC_GLOWFACTOR_STATIC )->SetText( sz );
	//	break;

	//case IDC_BLURFACTOR_SLIDER:
	//	floatNum = ( float )( RenderUI.GetSlider( IDC_BLURFACTOR_SLIDER )->GetValue() * 0.001f );
	//	swprintf_s( sz, 100, L"Blur Factor: %0.4f", floatNum );
	//	PostEffect_Blur.m_CBperResize.blur_factor = floatNum;
	//	RenderUI.GetStatic( IDC_BLURFACTOR_STATIC )->SetText( sz );
	//	break;

	//case IDC_GLOWBLENDFACTOR_SLIDER:
	//	floatNum = ( float )( RenderUI.GetSlider( IDC_GLOWBLENDFACTOR_SLIDER )->GetValue() * 0.01f );
	//	swprintf_s( sz, 100, L"Blend Factor: %0.2f", floatNum );
	//	PostEffect_Glow.m_CBperResize.blend_factor = floatNum;
	//	RenderUI.GetStatic( IDC_GLOWBLENDFACTOR_STATIC )->SetText( sz );
	//	break;
	}

}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );


	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen

	Initial();

	DXUTCreateWindow( L"A School of Fish" );

	// Only require 10-level hardware
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1280, 800 );
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

	// Perform any application-level cleanup here

	return DXUTGetExitCode();
}


