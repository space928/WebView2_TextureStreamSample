# WebView2_TextureStreamSample
This repository contains a minimal example of TextureStreams in WebView2 for the purpose of demonstrating a bug in the implementation.
This project creates a WebView2 and then creates a `ICoreWebView2ExperimentalTextureStream`; nothing else is done with the texture stream.

## Building
This project can be built using Visual Studio 2022 under MSVC v143.

This project has been tested on the following platform:
	* OS: Windows 10
	* SDK Version: Windows 10.0.19041.0
	* CPU: Intel i5 6600
	* GPU: Nvidia GTX 970
	* RAM: 16 Gb

On this platform `webView2Environment->CreateTextureStream(textureStreamID, d3dDevice, &textureStream)` returns an `HRESULT` of `ERROR_NOT_SUPPORTED (0x80070032)`.
