#pragma once
#include <d3d11.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>


HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);