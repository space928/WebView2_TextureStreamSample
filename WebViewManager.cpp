#include <string>

#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebViewManager.h"
#include "Logging.h"

using namespace Microsoft::WRL;

WebViewManager::WebViewManager(HWND hWnd, HINSTANCE hInst) : hWnd(hWnd), hInst(hInst)
{
		
}

void WebViewManager::Create(void (*onCreated)())
{
	SetEnvironmentVariableA("WEBVIEW2_RELEASE_CHANNEL_PREFERENCE", "1");
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this, onCreated](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				webviewEnvironment = env;
					
				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[this, onCreated, env](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							webviewController = controller;
							webviewController->get_CoreWebView2(&webview);
						}

						// Add a few settings for the webview
						// The demo step is redundant since the values are the default settings
						wil::com_ptr<ICoreWebView2Settings> settings;
						webview->get_Settings(&settings);
						settings->put_IsScriptEnabled(TRUE);
						settings->put_AreDefaultScriptDialogsEnabled(TRUE);
						settings->put_IsWebMessageEnabled(TRUE);
						settings->put_IsStatusBarEnabled(FALSE);
						settings->put_AreDefaultContextMenusEnabled(FALSE);

						// Resize WebView to fit the bounds of the parent window
						RECT bounds;
						GetClientRect(hWnd, &bounds);
						webviewController->put_Bounds(bounds);

						// Schedule an async task to navigate to a page
						webview->Navigate(GetLocalUri(L"webview2UIDemo.html").c_str());

						wil::unique_cotaskmem_string webViewVersion;
						env->get_BrowserVersionString(&webViewVersion);
						TEST_LOG(L"Initialised WebView! Version: %s", webViewVersion.get());

						EventRegistrationToken token;
						/*webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
							[](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
								wil::unique_cotaskmem_string uri;
								args->get_Uri(&uri);
								std::wstring source(uri.get());
								if (source.substr(0, 5) != L"https") {
									args->put_Cancel(true);
								}
								return S_OK;
							}).Get(), &token);*/

							// Step 6 - Communication between host and web content
							// Set an event handler for the host to return received message back to the web content
							/*webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
								[](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
									wil::unique_cotaskmem_string message;
									args->TryGetWebMessageAsString(&message);
									// processMessage(&message);
									webview->PostWebMessageAsString(message.get());
									return S_OK;
								}).Get(), &token);

							// Schedule an async task to add initialization script that
							// 1) Add an listener to print message from the host
							// 2) Post document URL to the host
							webview->AddScriptToExecuteOnDocumentCreated(
								L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
								L"window.chrome.webview.postMessage(window.document.URL);",
								nullptr);*/

						onCreated();

						return S_OK;
					}).Get());
				return S_OK;
			}).Get());
}

std::wstring WebViewManager::GetLocalPath(std::wstring relativePath, bool keepExePath)
{
	WCHAR rawPath[MAX_PATH];
	GetModuleFileNameW(hInst, rawPath, MAX_PATH);
	std::wstring path(rawPath);
	if (keepExePath)
	{
		path.append(relativePath);
	}
	else
	{
		std::size_t index = path.find_last_of(L"\\") + 1;
		path.replace(index, path.length(), relativePath);
	}
	return path;
}

std::wstring WebViewManager::GetLocalUri(std::wstring relativePath)
{
	std::wstring path = GetLocalPath(relativePath, false);

	wil::com_ptr<IUri> uri;
	if (FAILED(CreateUri(path.c_str(), Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME, 0, &uri)))
		return std::wstring();

	wil::unique_bstr uriBstr;
	if(FAILED(uri->GetAbsoluteUri(&uriBstr)))
		return std::wstring();
	return std::wstring(uriBstr.get());
}

void WebViewManager::OnResize(RECT bounds)
{
	if(webviewController != nullptr)
		webviewController->put_Bounds(bounds);
}