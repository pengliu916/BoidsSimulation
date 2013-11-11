#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "DXUTgui.h"
#define SUB_TEXTUREWIDTH 1024
#define SUB_TEXTUREHEIGHT 768

#include "MultiTexturePresenter.h"
#include "Shoal.h"


//--------------------------------------------------------------------------------------
//Global Variables
//--------------------------------------------------------------------------------------
MultiTexturePresenter						MultiTexture = MultiTexturePresenter(1, true, SUB_TEXTUREWIDTH, SUB_TEXTUREHEIGHT);
CDXUTDialogResourceManager					DialogResourceManager;
CDXUTDialog									UI;

Shoal										FishCluster(10240);
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

#define IDC_MAXFORCE_STATIC			15
#define IDC_MAXFORCE_SLIDER			16
#define IDC_MAXSPEED_STATIC			17
#define IDC_MAXSPEED_SLIDER			18
#define IDC_MINSPEED_STATIC			19
#define IDC_MINSPEED_SLIDER			20

#define IDC_FISHSIZE_STATIC			21
#define IDC_FISHSIZE_SLIDER			22

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

//--------------------------------------------------------------------------------------
//Initialization
//--------------------------------------------------------------------------------------
HRESULT Initial()
{
	HRESULT hr = S_OK;

	UI.Init(&DialogResourceManager);
	//UI.SetFont
	UI.SetCallback(OnGUIEvent); int iY = 10;

	UI.SetFont(1, L"Comic Sans MS", 400, 400);
	UI.SetFont(2, L"Courier New", 16, FW_NORMAL);

	WCHAR sz[100];
	iY += 24;
	swprintf_s(sz, 100, L"VisionDist: %0.2f", FishCluster.m_CBsimParams.fVisionDist);
	UI.AddStatic(IDC_VISIONDIST_STATIC, sz, 0, iY, 170, 23);
	UI.AddSlider(IDC_VISIONDIST_SLIDER, 0, iY += 26, 170, 23, 0, 200, 
				 (int)FishCluster.m_CBsimParams.fVisionDist*10);

	swprintf_s(sz, 100, L"VisionAngleCos: %0.2f", FishCluster.m_CBsimParams.fVisionAngleCos);
	UI.AddStatic(IDC_VISIONANGLE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_VISIONANGLE_SLIDER, 0, iY += 26, 170, 23, -100, 100, 
				 (int)(FishCluster.m_CBsimParams.fVisionAngleCos*100));

	swprintf_s(sz, 100, L"AvoidanceForce: %0.2f", FishCluster.m_CBsimParams.fAvoidanceFactor);
	UI.AddStatic(IDC_AVOIDANCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_AVOIDANCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
				 (int)(FishCluster.m_CBsimParams.fAvoidanceFactor * 10));

	swprintf_s(sz, 100, L"SeperationForce: %0.2f", FishCluster.m_CBsimParams.fSeperationFactor);
	UI.AddStatic(IDC_SEPERATIONFORCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_SEPERATIONFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 100, 
				 (int)(FishCluster.m_CBsimParams.fSeperationFactor*10));

	swprintf_s(sz, 100, L"CohesionForce: %0.2f", FishCluster.m_CBsimParams.fCohesionFactor);
	UI.AddStatic(IDC_COHESIONFORCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_COHESIONFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
				 (int)(FishCluster.m_CBsimParams.fCohesionFactor * 10));

	swprintf_s(sz, 100, L"AlignmentForce: %0.2f", FishCluster.m_CBsimParams.fAlignmentFactor);
	UI.AddStatic(IDC_ALIGNMENTFORCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_ALIGNMENTFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
				 (int)(FishCluster.m_CBsimParams.fAlignmentFactor * 10));

	swprintf_s(sz, 100, L"SeekingForce: %0.2f", FishCluster.m_CBsimParams.fSeekingFactor);
	UI.AddStatic(IDC_SEEKINGFORCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_SEEKINGFORCE_SLIDER, 0, iY += 26, 170, 23, 0, 100,
				 (int)(FishCluster.m_CBsimParams.fSeekingFactor * 10));

	swprintf_s(sz, 100, L"MaxForce: %0.2fs", FishCluster.m_CBsimParams.fMaxForce);
	UI.AddStatic(IDC_MAXFORCE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_MAXFORCE_SLIDER, 0, iY += 26, 170, 23, 10, 2000, 
				 (int)(FishCluster.m_CBsimParams.fMaxForce*10));

	swprintf_s(sz, 100, L"MaxSpeed: %0.2fs", FishCluster.m_CBsimParams.fMaxSpeed);
	UI.AddStatic(IDC_MAXSPEED_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_MAXSPEED_SLIDER, 0, iY += 26, 170, 23, 10, 2000,
				 (int)(FishCluster.m_CBsimParams.fMaxSpeed * 10));

	swprintf_s(sz, 100, L"MinSpeed: %0.2fs", FishCluster.m_CBsimParams.fMinSpeed);
	UI.AddStatic(IDC_MINSPEED_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_MINSPEED_SLIDER, 0, iY += 26, 170, 23, 0, 800, 
				 (int)(FishCluster.m_CBsimParams.fMinSpeed * 10));

	swprintf_s(sz, 100, L"FishSize: %0.2fs", FishCluster.m_fFishSize);
	UI.AddStatic(IDC_FISHSIZE_STATIC, sz, 0, iY += 26, 170, 23);
	UI.AddSlider(IDC_FISHSIZE_SLIDER, 0, iY += 26, 170, 23, 1, 200,
				 (int)(FishCluster.m_fFishSize * 10));
	
	FishCluster.Initial();
	V_RETURN(MultiTexture.Initial());
	return hr;
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	MultiTexture.ModifyDeviceSettings(pDeviceSettings);
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr = S_OK;
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));

	V_RETURN(FishCluster.CreateResource(pd3dDevice, pd3dImmediateContext));
	V_RETURN(MultiTexture.CreateResource(pd3dDevice, FishCluster.m_pOutputTextureRV));

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;
	V_RETURN(DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	UI.SetLocation(pBackBufferSurfaceDesc->Width - 180, 0);
	UI.SetSize(180, 600);

	MultiTexture.Resize();
	FishCluster.Resize();
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	FishCluster.Update(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	FishCluster.Render(pd3dImmediateContext);
	MultiTexture.Render(pd3dImmediateContext);

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR2, L"UI");
	UI.OnRender(fElapsedTime);
	DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{

	DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	FishCluster.Release();
	MultiTexture.Release();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = UI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	FishCluster.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	switch (nControlID)
	{
	case IDC_VISIONDIST_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float distFactor = (float)(UI.GetSlider(IDC_VISIONDIST_SLIDER)->GetValue() * 0.1f);
			swprintf_s(sz, 100, L"VisionDist: %0.2f", distFactor);
			FishCluster.m_CBsimParams.fVisionDist = distFactor;
			UI.GetStatic(IDC_VISIONDIST_STATIC)->SetText(sz);
			break;
							  }

	case IDC_VISIONANGLE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float angleFactor = (float)(UI.GetSlider(IDC_VISIONANGLE_SLIDER)->GetValue() * 0.01f);
			swprintf_s(sz, 100, L"VisionAngleCos: %0.2f", angleFactor);
			FishCluster.m_CBsimParams.fVisionAngleCos = angleFactor;
			UI.GetStatic(IDC_VISIONANGLE_STATIC)->SetText(sz);
			break;
								   }

	case IDC_AVOIDANCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float avoidanceFactor = (float)(UI.GetSlider(IDC_AVOIDANCE_SLIDER)->GetValue()*0.1f);
			swprintf_s(sz, 100, L"AvoidanceForce: %0.2f", avoidanceFactor);
			FishCluster.m_CBsimParams.fAvoidanceFactor = avoidanceFactor;
			UI.GetStatic(IDC_AVOIDANCE_STATIC)->SetText(sz);
			break;
								   }

	case IDC_SEPERATIONFORCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float seperationFactor = (float)(UI.GetSlider(IDC_SEPERATIONFORCE_SLIDER)->GetValue() * 0.1f);
			swprintf_s(sz, 100, L"SeperationForce: %0.2f", seperationFactor);
			FishCluster.m_CBsimParams.fSeperationFactor = seperationFactor;
			UI.GetStatic(IDC_SEPERATIONFORCE_STATIC)->SetText(sz);
			break;
							  }

	case IDC_COHESIONFORCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float cohesionFactor = (float)(UI.GetSlider(IDC_COHESIONFORCE_SLIDER)->GetValue() * 0.1f);
			swprintf_s(sz, 100, L"CohesionForce: %0.2f", cohesionFactor);
			FishCluster.m_CBsimParams.fCohesionFactor = cohesionFactor;
			UI.GetStatic(IDC_COHESIONFORCE_STATIC)->SetText(sz);
			break;
								}

	case IDC_ALIGNMENTFORCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float alignmentFactor = (float)(UI.GetSlider(IDC_ALIGNMENTFORCE_SLIDER)->GetValue()*0.1f);
			swprintf_s(sz, 100, L"AlignmentForce: %0.2f", alignmentFactor);
			FishCluster.m_CBsimParams.fAlignmentFactor = alignmentFactor;
			UI.GetStatic(IDC_ALIGNMENTFORCE_STATIC)->SetText(sz);
			break;
							}

	case IDC_SEEKINGFORCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float seekingFactor = (float)(UI.GetSlider(IDC_SEEKINGFORCE_SLIDER)->GetValue()*0.1f);
			swprintf_s(sz, 100, L"SeekingForce: %0.2f", seekingFactor);
			FishCluster.m_CBsimParams.fSeekingFactor = seekingFactor;
			UI.GetStatic(IDC_SEEKINGFORCE_STATIC)->SetText(sz);
			break;
							}
	case IDC_MAXFORCE_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float maxForceFactor = (float)(UI.GetSlider(IDC_MAXFORCE_SLIDER)->GetValue() * 0.1f);
			swprintf_s(sz, 100, L"MaxForce: %0.2fs", maxForceFactor);
			FishCluster.m_CBsimParams.fMaxForce = maxForceFactor;
			UI.GetStatic(IDC_MAXFORCE_STATIC)->SetText(sz);
			break;
								 }

	case IDC_MAXSPEED_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float maxSpeedFactor = (float)(UI.GetSlider(IDC_MAXSPEED_SLIDER)->GetValue() *0.1f);
			swprintf_s(sz, 100, L"MaxSpeed: %0.2fs", maxSpeedFactor);
			FishCluster.m_CBsimParams.fMaxSpeed = maxSpeedFactor;
			UI.GetStatic(IDC_MAXSPEED_STATIC)->SetText(sz);
			break;
								}

	case IDC_MINSPEED_SLIDER:
		{
			WCHAR sz[100]; FishCluster.m_bParamChanged = true;
			float minSpeedFactor = (float)(UI.GetSlider(IDC_MINSPEED_SLIDER)->GetValue() *0.1f);
			swprintf_s(sz, 100, L"MinSpeed: %0.2fs", minSpeedFactor);
			FishCluster.m_CBsimParams.fMinSpeed = minSpeedFactor;
			UI.GetStatic(IDC_MINSPEED_STATIC)->SetText(sz);
			break;
								  }
	case IDC_FISHSIZE_SLIDER:
		{
			WCHAR sz[100];
			float fishSize = (float)(UI.GetSlider(IDC_FISHSIZE_SLIDER)->GetValue() *0.1f);
			swprintf_s(sz, 100, L"MinSpeed: %0.2fs", fishSize);
			FishCluster.m_fFishSize = fishSize;
			UI.GetStatic(IDC_FISHSIZE_STATIC)->SetText(sz);
			break;
								}
	}

}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackMouse(OnMouse);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackDeviceRemoved(OnDeviceRemoved);


	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	// Perform any application-level initialization here

	DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen

	Initial();

	DXUTCreateWindow(L"A School of Fish");

	// Only require 10-level hardware
	DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1024, 768);
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

	// Perform any application-level cleanup here

	return DXUTGetExitCode();
}


