//**********************************************************************
//
// ShaderFramework.cpp
// 
// Super simple C-style framework for Shader Demo
// (NEVER ever write framework like this when you are making real games.)
//
// Author: Pope Kim
//
//**********************************************************************

#include "ShaderFramework.h"
#include <stdio.h>


//----------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------

// Constants
#define PI 3.14159265f
#define FOV (PI/4.0f) // Field of View
#define ASPECT_RATIO (WIN_WIDTH/(float)WIN_HEIGHT)
#define NEAR_PLANE 1
#define FAR_PLANE 10000

// D3D-related
LPDIRECT3D9             gpD3D = NULL;					// D3D
LPDIRECT3DDEVICE9       gpD3DDevice = NULL;				// D3D device
D3DXVECTOR4             gWorldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);
D3DXVECTOR4             gWorldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// Fonts
ID3DXFont*              gpFont = NULL;

// Models
LPD3DXMESH              gpDisc = NULL;
LPD3DXMESH              gpTorus = NULL;

// Surface Color
D3DXVECTOR4             gTorusColor(1, 1, 0, 1);
D3DXVECTOR4             gDiscColor(0, 1, 1, 1);

// Shadow Map Render Target
LPDIRECT3DTEXTURE9      gpShadowRenderTarget = NULL;
LPDIRECT3DSURFACE9      gpShadowDepthStencil = NULL;

// Shaders
LPD3DXEFFECT            gpApplyShadowShader = NULL;

// Light Color
D3DXVECTOR4             gLightColor(0.7f, 0.7f, 1.0f, 1.0f);

LPD3DXEFFECT            gpCreateShadowShader = NULL;



// Application name
const char*				gAppName = "Super Simple Shader Demo Framework";

// Rotation
float gRotY = 0.0f;


//-----------------------------------------------------------------------
// Application entry point/message loop
//-----------------------------------------------------------------------

// entry point
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    // register windows class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        gAppName, NULL };
    RegisterClassEx(&wc);

    // creates program window
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hWnd = CreateWindow(gAppName, gAppName,
        style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
        GetDesktopWindow(), NULL, wc.hInstance, NULL);

    // Client Rect size will be same as WIN_WIDTH and WIN_HEIGHT
    POINT ptDiff;
    RECT rcClient, rcWindow;

    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWindow);
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    // Initialize everything including D3D
    if (!InitEverything(hWnd))
        PostQuitMessage(1);

    // Message loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else // If there's no message to handle, update and draw the game
        {
            PlayDemo();
        }
    }

    UnregisterClass(gAppName, wc.hInstance);
    return 0;
}

// Message handler
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        ProcessInput(hWnd, wParam);
        break;

    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Keyboard input handler
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
    switch (keyPress)
    {
        // when ESC key is pressed, quit the demo
    case VK_ESCAPE:
        PostMessage(hWnd, WM_DESTROY, 0L, 0L);
        break;
    }
}

//------------------------------------------------------------
// game loop
//------------------------------------------------------------
void PlayDemo()
{
    Update();
    RenderFrame();
}

// Game logic update
void Update()
{
}

//------------------------------------------------------------
// Rendering
//------------------------------------------------------------

void RenderFrame()
{
    D3DCOLOR bgColor = 0xFF0000FF;	// background color - blue

    gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColor, 1.0f, 0);

    gpD3DDevice->BeginScene();
    {
        RenderScene();				// draw 3D objects and so on
        RenderInfo();				// show debug info
    }
    gpD3DDevice->EndScene();

    gpD3DDevice->Present(NULL, NULL, NULL, NULL);
}


// draw 3D objects and so on
void RenderScene()
{
    // create light-view matrix
    D3DXMATRIXA16 matLightView;
    {
        D3DXVECTOR3 vEyePt(gWorldCameraPosition.x, gWorldCameraPosition.y, gWorldCameraPosition.z);
        D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
        D3DXMatrixLookAtLH(&matLightView, &vEyePt, &vLookatPt, &vUpVec);
    }

    // create light-projection matrix
    D3DXMATRIXA16 matLightProjection;
    {
        D3DXMatrixPerspectiveFovLH(&matLightProjection, D3DX_PI / 4.0f, 1, 1, 1000);
    }



    // projection matrix
    D3DXMATRIXA16 matProjection;
    D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);


    D3DXMATRIXA16 matViewProjection;
    {
        // View Matrix
        D3DXMATRIXA16 matView;
        D3DXVECTOR3 vEyePt(gWorldCameraPosition.x, gWorldCameraPosition.y, gWorldCameraPosition.z);
        D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
        D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

        // Projection Matrix
        D3DXMATRIXA16 matProjection;

        D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

        D3DXMatrixMultiply(&matViewProjection, &matView, &matProjection);
    }

    gRotY += 0.4f * PI / 180.0f;

    if (gRotY > 2 * PI)
    {
        gRotY -= 2 * PI;
    }

    // World Matrix
    D3DXMATRIXA16 matTorusWorld;
    D3DXMatrixRotationY(&matTorusWorld, gRotY);

    // World Matrix for plane
    D3DXMATRIXA16 matDiscWorld;
    {
        D3DXMATRIXA16 matScale;
        D3DXMatrixScaling(&matScale, 2, 2, 2);

        D3DXMATRIXA16 matTrans;
        D3DXMatrixTranslation(&matTrans, 0, -40, 0);

        D3DXMatrixMultiply(&matDiscWorld, &matScale, &matTrans);
    }

    // current hardware backbuffer and depth buffer
    LPDIRECT3DSURFACE9  pHWBackBuffer = NULL;
    LPDIRECT3DSURFACE9  pHWDepthStencilBuffer = NULL;
    gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);

    gpD3DDevice->GetDepthStencilSurface(&pHWDepthStencilBuffer);

    // create shadow
    // use shadow render target and depth buffer
    LPDIRECT3DSURFACE9 pShadowSurface = NULL;
    HRESULT hr = gpShadowRenderTarget->GetSurfaceLevel(0, &pShadowSurface);
    if (SUCCEEDED(hr))
    {
        gpD3DDevice->SetRenderTarget(0, pShadowSurface);
        pShadowSurface->Release();
        pShadowSurface = NULL;
    }

    gpD3DDevice->SetDepthStencilSurface(gpShadowDepthStencil);

    // clears the shadow map from the last frame
    gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0xFFFFFFFF, 1.0f, 0);

    // set global variables for shadow creating shader
    gpCreateShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld);
    gpCreateShadowShader->SetMatrix("gLightViewMatrix", &matLightView);
    gpCreateShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection);

    UINT numPasses = 0;
    gpCreateShadowShader->Begin(&numPasses, NULL);

    for (UINT i = 0; i < numPasses; ++i)
    {
        gpCreateShadowShader->BeginPass(i);
        gpTorus->DrawSubset(0);
        gpCreateShadowShader->EndPass();
    }

    gpCreateShadowShader->End();


    // apply shadow
    // use hardware back buffer and depth buffer

    gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
    gpD3DDevice->SetDepthStencilSurface(pHWDepthStencilBuffer);

    pHWBackBuffer->Release();
    pHWBackBuffer = NULL;

    pHWDepthStencilBuffer->Release();
    pHWDepthStencilBuffer = NULL;

    ULONGLONG tick = GetTickCount64();

    gpApplyShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld);
    gpApplyShadowShader->SetMatrix("gViewProjectionMatrix", &matViewProjection);
   // gpApplyShadowShader->SetMatrix("gProjectionMatrix", &matProjection);
    gpApplyShadowShader->SetMatrix("gLightViewMatrix", &matLightView);
    gpApplyShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection);
    gpApplyShadowShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
    gpApplyShadowShader->SetVector("gObjectColor", &gTorusColor);
    gpApplyShadowShader->SetTexture("ShadowMap_Tex", gpShadowRenderTarget);

    //gpApplyShadowShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);
    //gpApplyShadowShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
    //gpApplyShadowShader->SetTexture("SpecularMap_Tex", gpStoneSM);
    //gpApplyShadowShader->SetFloat("gTime", tick / 1000.0f);
    //gpApplyShadowShader->SetFloat("gWaveHeight", 3.0f);
    //gpApplyShadowShader->SetFloat("gSpeed", 2.0f);
    //gpApplyShadowShader->SetFloat("gWaveFrequency", 10.0f);
    //gpApplyShadowShader->SetFloat("gUVSpeed", 0.25f);

    numPasses = 0;
    gpApplyShadowShader->Begin(&numPasses, NULL);

    for (UINT i = 0; i < numPasses; ++i)
    {
        gpApplyShadowShader->BeginPass(i);
        gpTorus->DrawSubset(0);

        gpApplyShadowShader->SetMatrix("gWorldMatrix", &matDiscWorld);
        gpApplyShadowShader->SetVector("gObjectColor", &gDiscColor);
        gpApplyShadowShader->CommitChanges();
        gpDisc->DrawSubset(0);
        gpApplyShadowShader->EndPass();
    }

    gpApplyShadowShader->End();
}

// show debug info
void RenderInfo()
{
    // text color
    D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

    // location to show the text
    RECT rct;
    rct.left = 5;
    rct.right = WIN_WIDTH / 3;
    rct.top = 5;
    rct.bottom = WIN_HEIGHT / 3;

    // show debug keys
    gpFont->DrawText(NULL, "Demo Framework\n\nESC: Quit demo", -1, &rct, 0, fontColor);
}

//------------------------------------------------------------
// initialization code
//------------------------------------------------------------
bool InitEverything(HWND hWnd)
{
    // init D3D
    if (!InitD3D(hWnd))
    {
        OutputDebugString("Failed to Init D3D.");

        return false;
    }

    // loading models, shaders and textures
    if (!LoadAssets())
    {
        OutputDebugString("Failed to load assets.");
        return false;
    }

    // load fonts
    if (FAILED(D3DXCreateFont(gpD3DDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE),
        "Arial", &gpFont)))
    {
        return false;
    }

    // create a render target
    const int shadowMapSize = 2048;

    HRESULT hr = gpD3DDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &gpShadowRenderTarget, NULL);

    if (FAILED(hr))
    {
        return false;
    }

    // also need to make a depth buffer that has the same size as the shadow map
    hr = gpD3DDevice->CreateDepthStencilSurface(shadowMapSize, shadowMapSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &gpShadowDepthStencil, NULL);

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

// D3D and device initialization
bool InitD3D(HWND hWnd)
{
    // D3D
    gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!gpD3D)
    {
        return false;
    }

    // fill in the strcture needed to create a D3D device
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.BackBufferWidth = WIN_WIDTH;
    d3dpp.BackBufferHeight = WIN_HEIGHT;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Windowed = TRUE;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
    d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    d3dpp.FullScreen_RefreshRateInHz = 0;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    // create D3D device
    if (FAILED(gpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &d3dpp, &gpD3DDevice)))
    {
        return false;
    }

    return true;
}

bool LoadAssets()
{
    // shader loading
    gpApplyShadowShader = LoadShader("ApplyShadow.fx");

    if (!gpApplyShadowShader)
    {
        return false;
    }

    gpCreateShadowShader = LoadShader("CreateShadow.fx");

    if (!gpCreateShadowShader)
    {
        return false;
    }

    // model loading
    gpTorus = LoadModel("torus.x");

    if (!gpTorus)
    {
        return false;
    }

    gpDisc = LoadModel("disc.x");

    if (!gpDisc)
    {
        return false;
    }

    return true;
}

// shader loading
LPD3DXEFFECT LoadShader(const char * filename)
{
    LPD3DXEFFECT ret = NULL;

    LPD3DXBUFFER pError = NULL;
    DWORD dwShaderFlags = 0;

#if _DEBUG
    dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

    D3DXCreateEffectFromFile(gpD3DDevice, filename,
        NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

    // if failed at loading shaders, display compile error
    // to output window
    if (!ret && pError)
    {
        int size = pError->GetBufferSize();
        void *ack = pError->GetBufferPointer();

        if (ack)
        {
            char* str = new char[size];
            sprintf(str, (const char*)ack, size);
            OutputDebugString(str);
            delete[] str;
        }
    }

    return ret;
}

// loading models
LPD3DXMESH LoadModel(const char * filename)
{
    LPD3DXMESH ret = NULL;
    if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, gpD3DDevice, NULL, NULL, NULL, NULL, &ret)))
    {
        OutputDebugString("failed at loading a model: ");
        OutputDebugString(filename);
        OutputDebugString("\n");
    };

    return ret;
}

// loading textures
LPDIRECT3DTEXTURE9 LoadTexture(const char * filename)
{
    LPDIRECT3DTEXTURE9 ret = NULL;
    if (FAILED(D3DXCreateTextureFromFile(gpD3DDevice, filename, &ret)))
    {
        OutputDebugString("failed at loading a texture: ");
        OutputDebugString(filename);
        OutputDebugString("\n");
    }

    return ret;
}
//------------------------------------------------------------
// cleanup code
//------------------------------------------------------------

void Cleanup()
{
    // release fonts
    if (gpFont)
    {
        gpFont->Release();
        gpFont = NULL;
    }

    // release models
    if (gpTorus)
    {
        gpTorus->Release();
        gpTorus = NULL;
    }

    if (gpDisc)
    {
        gpDisc->Release();
        gpDisc = NULL;
    }

    // release shaders
    if (gpApplyShadowShader)
    {
        gpApplyShadowShader->Release();
        gpApplyShadowShader = NULL;
    }

    if (gpCreateShadowShader)
    {
        gpCreateShadowShader->Release();
        gpCreateShadowShader = NULL;
    }


    // Release textures
    if (gpShadowRenderTarget)
    {
        gpShadowRenderTarget->Release();
        gpShadowRenderTarget = NULL;
    }

    if (gpShadowDepthStencil)
    {
        gpShadowDepthStencil->Release();
        gpShadowDepthStencil = NULL;
    }

    // release D3D
    if (gpD3DDevice)
    {
        gpD3DDevice->Release();
        gpD3DDevice = NULL;
    }

    if (gpD3D)
    {
        gpD3D->Release();
        gpD3D = NULL;
    }
}

