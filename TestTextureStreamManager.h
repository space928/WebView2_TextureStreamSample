#pragma once

#include "framework.h"
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebViewManager.h"
#include "WebView2Experimental.h"
#include <d3dcommon.h>
#include <dxgi.h>
#include <d3d11.h>

using namespace Microsoft::WRL;

class TestTextureStreamManager
{
private:
	ICoreWebView2ExperimentalEnvironment12* webView2Environment;
	ICoreWebView2ExperimentalTextureStream* textureStream;
	IDXGIFactory1* dxgiFactory;
	ID3D11Device* d3dDevice;
	D3D_FEATURE_LEVEL d3dFeatureLevel;
	ID3D11DeviceContext* d3dDeviceContext;
	EventRegistrationToken eventToken;
	const LPCWSTR textureStreamID = L"testTextureStream0";

public:
	TestTextureStreamManager();
	~TestTextureStreamManager();
	BOOL Initialise(ICoreWebView2Environment* webView2Env);

private:
	HRESULT CreateD3DDevice(UINT64 gpuAdapterLUID);
	HRESULT CreateTextureStream();
	void CleanupDevice();
};

