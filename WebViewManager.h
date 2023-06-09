#pragma once

#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebViewManager.h"
#include <string>

using namespace Microsoft::WRL;

class WebViewManager
{
private:
	// Pointer to WebViewController
	wil::com_ptr<ICoreWebView2Controller> webviewController;

	// Pointer to WebView window
	wil::com_ptr<ICoreWebView2> webview;
	ICoreWebView2Environment* webviewEnvironment;

	HWND hWnd;
	HINSTANCE hInst;

public:
	inline ICoreWebView2Controller* GetController()
	{
		return webviewController.get();
	}

	WebViewManager(HWND hWnd, HINSTANCE hInst);

	void Create(void (*onCreated)());

	void OnResize(RECT bounds);
	inline ICoreWebView2Environment* GetEnvironment() 
	{
		if (webviewEnvironment != nullptr && webviewController != nullptr)
			return webviewEnvironment;

		return nullptr;
	}

private:
	std::wstring GetLocalPath(std::wstring relativePath, bool keepExePath);
	std::wstring GetLocalUri(std::wstring relativePath);
};
