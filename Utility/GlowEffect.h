#pragma once

#include <D3D11.h>
#include <DirectXMath.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 640
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 480
#endif

using namespace std;

struct GE_ConstBuffer
{
	UINT DepthWidth;
	UINT DepthHeight;
	float glow_factor;
	float blend_factor;
};

class GlowEffect
{
public:
	ID3D11VertexShader*				m_pVertexShader;
	ID3D11PixelShader*				m_pPixelShader_H;
	ID3D11PixelShader*				m_pPixelShader_V;
	ID3D11PixelShader*				m_pPixelShader_ALL;
	ID3D11GeometryShader*			m_pGeometryShader;
	ID3D11InputLayout*				m_pVertexLayout;
	ID3D11Buffer*					m_pVertexBuffer; 

	ID3D11ShaderResourceView*		m_pInputTextureSRV;
	ID3D11SamplerState*				m_pGeneralTexSS;

	ID3D11Texture2D*				m_pGlow_H_Texture2D;
	ID3D11Texture2D*				m_pGlow_HV_Texture2D;
	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11RenderTargetView*			m_pGlow_H_TextureRTV;
	ID3D11RenderTargetView*			m_pGlow_HV_TextureRTV;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pGlow_H_TextureSRV; 
	ID3D11ShaderResourceView*		m_pGlow_HV_TextureSRV; 
	ID3D11ShaderResourceView*		m_pOutputTextureSRV; 

	ID3D11BlendState*				m_pOutputBlendState;


	D3D11_VIEWPORT					m_RTviewport;

	CModelViewerCamera*				m_pCamera;


	GE_ConstBuffer					m_CBperResize;
	ID3D11Buffer*					m_pCBperResize;

	UINT			m_rendertargetWidth;
	UINT			m_rendertargetHeight;

	UINT			m_renderGlowTargetWidth;
	UINT			m_renderGlowTargetHeight;


	GlowEffect(UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT)
	{
		m_rendertargetWidth=width;
		m_rendertargetHeight=height;

		m_renderGlowTargetWidth = (UINT)(m_rendertargetWidth / 8.0);
		m_renderGlowTargetHeight = (UINT)(m_rendertargetHeight / 8.0);

		m_CBperResize.DepthWidth = m_rendertargetWidth;
		m_CBperResize.DepthHeight = m_rendertargetHeight;
		m_CBperResize.glow_factor = 4.5;
		m_CBperResize.blend_factor = 1;
	}

	HRESULT Initial()
	{
		HRESULT hr=S_OK;
		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice,
		ID3D11ShaderResourceView* pInputTextureSRV, CModelViewerCamera* pCamera)
	{
		HRESULT hr=S_OK;
		m_pCamera = pCamera;

		ID3DBlob* pVSBlob = NULL;
		wstring filename=L"..\\Utility\\GlowEffect.fx";
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "VS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pVertexShader));

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "GS", "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(),NULL,&m_pGeometryShader));
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_H", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pPixelShader_H));
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_V", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pPixelShader_V));
		V_RETURN(DXUTCompileFromFile(filename.c_str(), nullptr, "PS_Glow_ALL", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pPixelShader_ALL));
		pPSBlob->Release();

		D3D11_INPUT_ELEMENT_DESC layout[] = { { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
		V_RETURN(pd3dDevice->CreateInputLayout(layout,ARRAYSIZE(layout),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pVertexLayout));
		pVSBlob->Release();

		m_pInputTextureSRV=pInputTextureSRV;

		// Create the vertex buffer
		D3D11_BUFFER_DESC bd = {0};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(short);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

		// Create the constant buffers
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0    ;
		bd.ByteWidth = sizeof(GE_ConstBuffer);
		V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pCBperResize ));
		DXUT_SetDebugName( m_pCBperResize, "m_pCBperResize");

		//Create rendertaget resource
		D3D11_TEXTURE2D_DESC	RTtextureDesc = {0};
		RTtextureDesc.Width=m_renderGlowTargetWidth;
		RTtextureDesc.Height=m_renderGlowTargetHeight;


		RTtextureDesc.MipLevels=1;
		RTtextureDesc.ArraySize=1;
		RTtextureDesc.Format=DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTtextureDesc.SampleDesc.Count=1;
		RTtextureDesc.Usage=D3D11_USAGE_DEFAULT;
		RTtextureDesc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
		RTtextureDesc.CPUAccessFlags=0;
		RTtextureDesc.MiscFlags=0;

		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc,NULL,&m_pGlow_H_Texture2D));
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc,NULL,&m_pGlow_HV_Texture2D));

		D3D11_SHADER_RESOURCE_VIEW_DESC RTshaderResourceDesc;
		RTshaderResourceDesc.Format=RTtextureDesc.Format;
		RTshaderResourceDesc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
		RTshaderResourceDesc.Texture2D.MostDetailedMip=0;
		RTshaderResourceDesc.Texture2D.MipLevels=1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pGlow_H_Texture2D,&RTshaderResourceDesc,&m_pGlow_H_TextureSRV));
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pGlow_HV_Texture2D,&RTshaderResourceDesc,&m_pGlow_HV_TextureSRV));

		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format=RTtextureDesc.Format;
		RTviewDesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice=0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pGlow_H_Texture2D,&RTviewDesc,&m_pGlow_H_TextureRTV));
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pGlow_HV_Texture2D,&RTviewDesc,&m_pGlow_HV_TextureRTV));

		RTtextureDesc.Width=m_rendertargetWidth;
		RTtextureDesc.Height=m_rendertargetHeight;
		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc,NULL,&m_pOutputTexture2D));

		RTshaderResourceDesc.Format=RTtextureDesc.Format;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutputTexture2D,&RTshaderResourceDesc,&m_pOutputTextureSRV));
		
		RTviewDesc.Format=RTtextureDesc.Format;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutputTexture2D,&RTviewDesc,&m_pOutputTextureRTV));

		// Create the blend state
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC) );
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;        
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA         ;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE        ;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE    ;        ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE   ;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL ;
		//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F ;
		V_RETURN( pd3dDevice->CreateBlendState( &blendDesc, &m_pOutputBlendState ));

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pGeneralTexSS ));

		m_RTviewport.Width=(float)m_rendertargetWidth;
		m_RTviewport.Height=(float)m_rendertargetHeight;
		m_RTviewport.MinDepth=0.0f;
		m_RTviewport.MaxDepth=1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		return hr;
	}

	~GlowEffect()
	{

	}

	void Release()
	{
		SAFE_RELEASE(m_pVertexShader);
		SAFE_RELEASE(m_pPixelShader_H);
		SAFE_RELEASE(m_pPixelShader_V);
		SAFE_RELEASE(m_pPixelShader_ALL);
		SAFE_RELEASE(m_pGeometryShader);
		SAFE_RELEASE(m_pVertexLayout);
		SAFE_RELEASE(m_pVertexBuffer);

		//SAFE_RELEASE(m_pOutputTextureSRV);
		SAFE_RELEASE(m_pGlow_H_Texture2D);
		SAFE_RELEASE(m_pGlow_H_TextureRTV);
		SAFE_RELEASE(m_pGlow_H_TextureSRV);
		SAFE_RELEASE(m_pGlow_HV_Texture2D);
		SAFE_RELEASE(m_pGlow_HV_TextureRTV);
		SAFE_RELEASE(m_pGlow_HV_TextureSRV);
		SAFE_RELEASE(m_pOutputTexture2D);
		SAFE_RELEASE(m_pOutputTextureSRV);
		SAFE_RELEASE(m_pOutputTextureRTV);

		SAFE_RELEASE(m_pCBperResize);
		SAFE_RELEASE(m_pOutputBlendState);

		SAFE_RELEASE(m_pGeneralTexSS);


	}
	void Render(ID3D11DeviceContext* pd3dImmediateContext)
	{
			pd3dImmediateContext->IASetInputLayout(m_pVertexLayout);
			pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			UINT stride = 0;
			UINT offset = 0;
			pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
			pd3dImmediateContext->OMSetRenderTargets(1,&m_pGlow_H_TextureRTV,NULL);
			pd3dImmediateContext->OMSetBlendState( m_pOutputBlendState, NULL, 0xffffffff );

			pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pCBperResize );
			pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &m_pCBperResize );
			pd3dImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
			pd3dImmediateContext->GSSetShader(m_pGeometryShader,NULL,0);	
			pd3dImmediateContext->PSSetShader( m_pPixelShader_H, NULL, 0 );
			pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pInputTextureSRV);

			m_RTviewport.Height = (FLOAT)m_renderGlowTargetHeight;
			m_RTviewport.Width = (FLOAT)m_renderGlowTargetWidth;
			pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
			
			m_CBperResize.DepthWidth = m_renderGlowTargetWidth;
			m_CBperResize.DepthHeight = m_renderGlowTargetHeight;

			pd3dImmediateContext->UpdateSubresource( m_pCBperResize, 0, NULL, &m_CBperResize, 0, 0 );

			pd3dImmediateContext->Draw(1,0);	

			pd3dImmediateContext->OMSetRenderTargets(1,&m_pGlow_HV_TextureRTV,NULL);
			pd3dImmediateContext->PSSetShader( m_pPixelShader_V, NULL, 0 );
			pd3dImmediateContext->PSSetShaderResources(1, 1, &m_pGlow_H_TextureSRV);
			pd3dImmediateContext->Draw(1,0);	

			float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			//pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,m_pOutputStencilView);
			pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,NULL);
			pd3dImmediateContext->ClearRenderTargetView( m_pOutputTextureRTV, ClearColor );

			pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,NULL);
			pd3dImmediateContext->PSSetShader( m_pPixelShader_ALL, NULL, 0 );
			pd3dImmediateContext->PSSetShaderResources(2, 1, &m_pGlow_HV_TextureSRV);
			pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS);

			m_RTviewport.Height = (FLOAT)m_rendertargetHeight;
			m_RTviewport.Width = (FLOAT)m_rendertargetWidth;
			pd3dImmediateContext->RSSetViewports(1, &m_RTviewport);
			
			m_CBperResize.DepthWidth = m_rendertargetWidth;
			m_CBperResize.DepthHeight = m_rendertargetHeight;

			pd3dImmediateContext->UpdateSubresource( m_pCBperResize, 0, NULL, &m_CBperResize, 0, 0 );

			pd3dImmediateContext->Draw(1,0);

			ID3D11ShaderResourceView* ppSRVNULL[3] = { NULL,NULL,NULL };
			pd3dImmediateContext->PSSetShaderResources( 0, 3, ppSRVNULL );

	}

	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);
		UNREFERENCED_PARAMETER(hWnd);

		return 0;
	}
};