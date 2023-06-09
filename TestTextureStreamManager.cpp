#include "TestTextureStreamManager.h"
#include "framework.h"
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebViewManager.h"
#include "WebView2Experimental.h"
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>

using namespace Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d10_1.lib")
#pragma comment(lib, "dxgi.lib")

TestTextureStreamManager::TestTextureStreamManager() : 
	webView2Environment(nullptr), textureStream(nullptr)
{
	
}

TestTextureStreamManager::~TestTextureStreamManager()
{
	CleanupDevice();
}

BOOL TestTextureStreamManager::Initialise(ICoreWebView2Environment* webView2Env)
{
	if (webView2Env == nullptr)
		return FALSE;
	if (webView2Env->QueryInterface<ICoreWebView2ExperimentalEnvironment12>(&webView2Environment) == E_NOINTERFACE)
		return FALSE;

	HRESULT hr;
	UINT64 gpuLUID = 0;
	hr = webView2Environment->add_RenderAdapterLUIDChanged(Callback<ICoreWebView2ExperimentalRenderAdapterLUIDChangedEventHandler>(
		[this](ICoreWebView2ExperimentalEnvironment12* webView, IUnknown* args) -> HRESULT {
			return CreateTextureStream(); 
	}).Get(), &eventToken);
	if (hr != S_OK)
		return FALSE;

	// Try to create the texture stream immeadiately if the render adapter is available, otherwise, wait for the callback
	hr = webView2Environment->get_RenderAdapterLUID(&gpuLUID);
	if (hr == S_OK && gpuLUID != 0)
		return CreateTextureStream() == S_OK;

	return TRUE;
}

HRESULT TestTextureStreamManager::CreateTextureStream()
{
	HRESULT hr = S_OK;
	UINT64 gpuLUID = 0;
	hr = webView2Environment->get_RenderAdapterLUID(&gpuLUID);
	if (hr != S_OK)
	{
		TEST_LOG(L"Failed to get render adapter LUID hr: 0x%x", hr);
		return hr;
	}
	hr = CreateD3DDevice(gpuLUID);
	if (hr != S_OK)
	{
		TEST_LOG(L"Failed to create d3d device hr: 0x%x", hr);
		return hr;
	}
	hr = webView2Environment->CreateTextureStream(textureStreamID, d3dDevice, &textureStream);
	if (hr != S_OK)
	{
		TEST_LOG(L"Failed to create texture stream hr: 0x%x", hr);
		return hr;
	}

	return hr;
}

HRESULT TestTextureStreamManager::CreateD3DDevice(UINT64 gpuAdapterLUID)
{
	SetEnvironmentVariableA("SHIM_MCCOMPAT", "0x800000000");
	HRESULT hr = S_OK;

	IDXGIAdapter* desiredAdapter = nullptr;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), __out(void**) &dxgiFactory);
	if (FAILED(hr))
		return hr;

	for (unsigned i = 0; dxgiFactory->EnumAdapters(i, __out & desiredAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		hr = desiredAdapter->GetDesc(__out & adapterDesc);

		union {
			LUID luid;
			unsigned long long luidAsInt64;
		};
		luid = adapterDesc.AdapterLuid;
		if (gpuAdapterLUID == luidAsInt64)
			break;

		desiredAdapter->Release();
		desiredAdapter = nullptr;
	}

	//////////////////////////////////////////////////////
	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType = desiredAdapter != nullptr ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
	hr = D3D11CreateDevice(desiredAdapter, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, __out & d3dDevice, __out & d3dFeatureLevel, __out & d3dDeviceContext);

	desiredAdapter->Release();

	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TestTextureStreamManager::CleanupDevice()
{
	if (webView2Environment) webView2Environment->remove_RenderAdapterLUIDChanged(eventToken);
	if (textureStream) textureStream->Stop();
	if (d3dDeviceContext) d3dDeviceContext->ClearState();
	if (dxgiFactory) dxgiFactory->Release();
	if (d3dDevice) d3dDevice->Release();
}
