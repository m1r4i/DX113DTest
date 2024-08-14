#include "ObjLoader.h"
#include "Shader.h"
#include <iostream>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <algorithm>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#pragma warning(disable: 4996)

#define Window_Width 1000
#define Window_Height 800

ID3D11Device* pDevice;
ID3D11DeviceContext* pContext;
IDXGISwapChain* pSwapChain;
ID3D11RenderTargetView* pRTV;

ID3D11Buffer* pVertexBuffer;
ID3D11Buffer* pIndexBuffer;

ID3D11VertexShader* pVertexShader;
ID3D11PixelShader* pPixelShader;
ID3D11InputLayout* pInputLayout;
ID3D11Buffer* pConstantBuffer;

ID3D11ShaderResourceView* pTextureRV = nullptr;
ID3D11SamplerState* pSamplerLinear = nullptr;

DirectX::XMFLOAT3 eye = { 0.0f, 50.0f, -80.0f };
DirectX::XMFLOAT3 lookat = { 0.0f, 0.0f, 0.0f };
DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
float moveSpeed = 0.1f;
float zoomSpeed = 0.1f;
float lotate = 0.0f;
DirectX::XMFLOAT4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
DirectX::XMFLOAT3 lightLocation = { 1.0f, 1.0f, 1.0f };
float lightAmbient = { 0.3f };

struct ConstantBuffer {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX inWorld;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMFLOAT4 lightCol;
    DirectX::XMFLOAT3 lightLoc;
    float lightAmbient;
    DirectX::XMFLOAT3 eyePos;
    float padding;
};


enum EMODE {
    CAM,
    LIGHT,
    MODEL
};

enum CMODE {
    R,
    G,
    B
};

EMODE mode = CAM;
CMODE cmode = R;

void ProcessModeChange() {
    if (GetAsyncKeyState(VK_F1) & 0x8000) { mode = CAM; } // F1
    if (GetAsyncKeyState(VK_F2) & 0x8000) { mode = LIGHT; } // F1
    if (GetAsyncKeyState(VK_F3) & 0x8000) { mode = MODEL; } // F1
}

void ProcessColorModeChange() {
    if (GetAsyncKeyState(51) & 0x8000) { cmode = R; } // F1
    if (GetAsyncKeyState(52) & 0x8000) { cmode = G; } // F1
    if (GetAsyncKeyState(53) & 0x8000) { cmode = B; } // F1
}

void ProcessCameraInput() {
    if (GetAsyncKeyState(65) & 0x8000) { eye.x -= moveSpeed; lookat.x -= moveSpeed; } // A
    if (GetAsyncKeyState(68) & 0x8000) { eye.x += moveSpeed; lookat.x += moveSpeed; } // D
    if (GetAsyncKeyState(87) & 0x8000) { eye.y += moveSpeed; lookat.y += moveSpeed; } // W
    if (GetAsyncKeyState(83) & 0x8000) { eye.y -= moveSpeed; lookat.y -= moveSpeed; } // S
    if (GetAsyncKeyState(49) & 0x8000) { eye.z += zoomSpeed; } // 1
    if (GetAsyncKeyState(50) & 0x8000) { eye.z -= zoomSpeed; } // 2
}

void ProcessLightInput() {
    if (GetAsyncKeyState(65) & 0x8000) { lightLocation.x -= 0.1f; } // A
    if (GetAsyncKeyState(68) & 0x8000) { lightLocation.x += 0.1f; } // D
    if (GetAsyncKeyState(87) & 0x8000) { lightLocation.y += 0.1f; } // W
    if (GetAsyncKeyState(83) & 0x8000) { lightLocation.y -= 0.1f; } // S
    if (GetAsyncKeyState(49) & 0x8000) { lightLocation.z += 0.1f; } // 1
    if (GetAsyncKeyState(50) & 0x8000) { lightLocation.z -= 0.1f; } // 2
    if (GetAsyncKeyState(82) & 0x8000) { lightAmbient = (std::min)(lightAmbient + 0.00392f, 3.0f); } // R
    if (GetAsyncKeyState(84) & 0x8000) { lightAmbient = (std::max)(lightAmbient - 0.00392f, 0.0f); } // T
}

void ProcessColorInput() {
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        switch (cmode) {
        case R: lightColor.x = (std::min)(lightColor.x + 0.00392f, 1.0f); break;
        case G: lightColor.y = (std::min)(lightColor.y + 0.00392f, 1.0f); break;
        case B: lightColor.z = (std::min)(lightColor.z + 0.00392f, 1.0f); break;
        }
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        switch (cmode) {
        case R: lightColor.x = (std::max)(lightColor.x - 0.00392f, 0.0f); break;
        case G: lightColor.y = (std::max)(lightColor.y - 0.00392f, 0.0f); break;
        case B: lightColor.z = (std::max)(lightColor.z - 0.00392f, 0.0f); break;
        }
    }
}

void ProcessInput() {
    switch (mode) {
    case CAM: ProcessCameraInput(); break;
    case LIGHT: ProcessLightInput(); break;
    case MODEL: /* モデル操作の処理を追加 */ break;
    }
    ProcessColorInput();
    ProcessModeChange();
    ProcessColorModeChange();
    // その他の入力処理

    if (GetAsyncKeyState(81) & 0x8000) {
        lotate -= 0.1f;
    } // Q
    if (GetAsyncKeyState(69) & 0x8000) {
        lotate += 0.1f;
    } // E
}


void UpdateViewMatrix(ConstantBuffer& cb) {
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookat), XMLoadFloat3(&up));
    cb.view = XMMatrixTranspose(viewMatrix);
    pContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEX w = {};
    w.cbSize = sizeof(WNDCLASSEX);
    w.lpfnWndProc = (WNDPROC)WndProc;
    w.lpszClassName = "DX11Sample";
    w.hInstance = hInstance;

    RegisterClassEx(&w);

    RECT wrc = { 0, 0, Window_Width, Window_Height };
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    HWND hwnd = CreateWindow(w.lpszClassName, "DX11テスト", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, w.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    model m;
    SetWindowText(hwnd, "Loading model...");
    loadObj("assets/rem-solo.obj", &m);

    SetWindowText(hwnd, "Preparing DirectX11...");

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = Window_Width;
    sd.BufferDesc.Height = Window_Height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 7, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, nullptr, &pContext);

    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRTV);
    pBackBuffer->Release();
    pContext->OMSetRenderTargets(1, &pRTV, nullptr);

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)Window_Width;
    vp.Height = (FLOAT)Window_Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pContext->RSSetViewports(1, &vp);

    SetWindowText(hwnd, "Preparing Vertices...");

    // 頂点バッファの作成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertex) * m.v.count;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = m.v.vertices;
    pDevice->CreateBuffer(&bd, &initData, &pVertexBuffer);

    // インデックスバッファの作成
    int* indices = (int*)malloc(sizeof(int) * m.f.count * 3);
    for (int i = 0; i < m.f.count; ++i) {
        indices[i * 3] = m.f.faces[i].vertexIndices[0] - 1;
        indices[i * 3 + 1] = m.f.faces[i].vertexIndices[1] - 1;
        indices[i * 3 + 2] = m.f.faces[i].vertexIndices[2] - 1;
    }

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(int) * m.f.count * 3;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    initData.pSysMem = indices;
    pDevice->CreateBuffer(&bd, &initData, &pIndexBuffer);

    free(indices);

    SetWindowText(hwnd, "Preparing Shaders...");

    // シェーダーの初期化
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(L"shader2.hlsl", "VS", "vs_5_0", &pVSBlob);
    if (FAILED(hr)) {
        MessageBox(nullptr, "The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
        return hr;
    }
    hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
    if (FAILED(hr)) {
        pVSBlob->Release();
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);
    hr = pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pInputLayout);
    pVSBlob->Release();
    if (FAILED(hr)) return hr;

    pContext->IASetInputLayout(pInputLayout);

    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader2.hlsl", "PS", "ps_5_0", &pPSBlob);
    if (FAILED(hr)) {
        MessageBox(nullptr, "The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
        return hr;
    }
    hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr)) return hr;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = pDevice->CreateBuffer(&bd, nullptr, &pConstantBuffer);
    if (FAILED(hr)) return hr;

    SetWindowText(hwnd, "Preparing Cam and Matrixes...");

    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookat), XMLoadFloat3(&up));

    float fov = DirectX::XM_PIDIV2; // 90度
    float aspectRatio = static_cast<float>(Window_Width) / static_cast<float>(Window_Height);
    float nearZ = 0.01f;
    float farZ = 10000.0f;

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

    // ワールド行列の逆行列を計算
    DirectX::XMMATRIX worldInverseTransposeMatrix = DirectX::XMMatrixInverse(nullptr, worldMatrix);


    ConstantBuffer cb;
    cb.world = XMMatrixTranspose(worldMatrix);
    cb.inWorld = worldInverseTransposeMatrix;
    cb.view = XMMatrixTranspose(viewMatrix);
    cb.projection = XMMatrixTranspose(projectionMatrix);
    cb.lightAmbient = lightAmbient;
    cb.lightLoc = lightLocation;
    cb.lightCol = lightColor;
    cb.eyePos = eye;
    pContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
    pContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
    pContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);


    // サンプラーの作成
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    pDevice->CreateSamplerState(&sampDesc, &pSamplerLinear);


    // グレースケールのテクスチャを生成 (例: 1x1の灰色テクスチャ)
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    ID3D11Texture2D* pGrayTexture = nullptr;
    pDevice->CreateTexture2D(&texDesc, nullptr, &pGrayTexture);

    // テクスチャデータの設定
    unsigned char grayColor[4] = { 128, 128, 128, 255 }; // 灰色
    pContext->UpdateSubresource(pGrayTexture, 0, nullptr, grayColor, 0, 0);

    // Shader Resource View の作成
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    pDevice->CreateShaderResourceView(pGrayTexture, &srvDesc, &pTextureRV);


    // コンテキストにサンプラーとテクスチャを設定
    pContext->PSSetSamplers(0, 1, &pSamplerLinear);
    pContext->PSSetShaderResources(0, 1, &pTextureRV);

    // 深度ステンシルバッファの作成
    ID3D11Texture2D* depthStencilBuffer = nullptr;
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = Window_Width;
    depthStencilDesc.Height = Window_Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;

    hr = pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, "Failed to create depth stencil buffer", "Error", MB_OK);
        return hr;
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;  // 近いピクセルを優先して描画

    ID3D11DepthStencilState* pDSState;
    pDevice->CreateDepthStencilState(&dsDesc, &pDSState);
    pContext->OMSetDepthStencilState(pDSState, 1);

    // 深度ステンシルビューの作成
    ID3D11DepthStencilView* pDepthStencilView = nullptr;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = pDevice->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &pDepthStencilView);
    if (FAILED(hr)) {
        MessageBox(nullptr, "Failed to create depth stencil view", "Error", MB_OK);
        return hr;
    }

    // レンダーターゲットと深度ステンシルビューを設定
    pContext->OMSetRenderTargets(1, &pRTV, pDepthStencilView);

    // 深度ステンシルバッファのクリア
    float depthClear = 1.0f;
    UINT8 stencilClear = 0;
    pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClear, stencilClear);



    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            break;
        }

        char w[256];
        if (mode == CAM) {
            sprintf(w, "In rendering | Cam> X: %f, Y: %f, Z: %f", eye.x, eye.y, eye.z);
        }
        else if (mode == LIGHT) {
			sprintf(w, "In rendering | Light> X: %f, Y: %f, Z: %f, Amb: %f Color: R%f G%f B%f", lightLocation.x, lightLocation.y, lightLocation.z, lightAmbient, lightColor.x, lightColor.y, lightColor.z);
		}
		else if (mode == MODEL) {
			sprintf(w, "In rendering | Model> ");
		}
        SetWindowText(hwnd, w);

        ProcessInput();

        cb.world = XMMatrixTranspose(DirectX::XMMatrixIdentity());
        float angle = DirectX::XMConvertToRadians(lotate);
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(angle);
        cb.world = cb.world * rotationMatrix;
        // ワールド行列の逆行列を計算
        DirectX::XMMATRIX worldInverseTransposeMatrix = DirectX::XMMatrixInverse(nullptr, cb.world);

        cb.inWorld = worldInverseTransposeMatrix;
        cb.projection = XMMatrixTranspose(projectionMatrix);
        cb.lightAmbient = lightAmbient;
        cb.lightLoc = lightLocation;
        cb.lightCol = lightColor;
        cb.eyePos = eye;
        UpdateViewMatrix(cb);

        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
        pContext->ClearRenderTargetView(pRTV, ClearColor);

        UINT stride = sizeof(vertex);
        UINT offset = 0;
        pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
        pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        pContext->VSSetShader(pVertexShader, nullptr, 0);
        pContext->PSSetShader(pPixelShader, nullptr, 0);

        pContext->DrawIndexed(m.f.count * 3, 0, 0);

        pSwapChain->Present(0, 0);
    }

    UnregisterClass(w.lpszClassName, w.hInstance);

    free(m.v.vertices);
    free(m.n.normals);
    free(m.f.faces);
    if (pVertexBuffer) pVertexBuffer->Release();
    if (pIndexBuffer) pIndexBuffer->Release();
    if (pRTV) pRTV->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pContext) pContext->Release();
    if (pDevice) pDevice->Release();

    return 0;
}
