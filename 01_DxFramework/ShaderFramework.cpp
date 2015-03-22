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

// fullscreen quad
LPDIRECT3DVERTEXDECLARATION9 gpFullscreenQuadDecl = NULL;
LPDIRECT3DVERTEXBUFFER9 gpFullscreenQuadVB = NULL;
LPDIRECT3DINDEXBUFFER9 gpFullscreenQuadIB = NULL;

// Fonts
ID3DXFont*              gpFont = NULL;

// Models
LPD3DXMESH              gpTeapot = NULL;

// Surface Color
D3DXVECTOR4             gpTeapotColor(1, 1, 0, 1);

// Shaders
LPD3DXEFFECT            gpEnvironmentMappingShader = NULL;
LPD3DXEFFECT            gpNoEffect = NULL;
LPD3DXEFFECT            gpGrayScale = NULL;
LPD3DXEFFECT            gpSepia = NULL;

// Light Color
D3DXVECTOR4             gLightColor(0.7f, 0.7f, 1.0f, 1.0f);

// Textures
LPDIRECT3DTEXTURE9      gpTeapotDM = NULL;
LPDIRECT3DTEXTURE9      gpTeapotNM = NULL;
LPDIRECT3DTEXTURE9      gpTeapotSM = NULL;
LPDIRECT3DCUBETEXTURE9  gpSnowENV = NULL;

// scene render target
LPDIRECT3DTEXTURE9      gpSceneRenderTarget = NULL;

// Application name
const char*				gAppName = "Super Simple Shader Demo Framework";

// Rotation
float gRotY = 0.0f;

// index of postprocess shader to use
int gPostProcessIndex = 0;

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
    case '1':
    case '2':
    case '3':
        gPostProcessIndex = keyPress - '0' - 1;
        break;
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
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY(&matWorld, gRotY);

    D3DXMATRIXA16 matWorldViewProjection;
    D3DXMatrixMultiply(&matWorldViewProjection, &matViewProjection, &matWorld);

    // 1. draw the scene into the render target
    // current hardware backbuffer
    LPDIRECT3DSURFACE9  pHWBackBuffer = NULL;
    gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);

    // draw onto the render target 
    LPDIRECT3DSURFACE9  pSceneSurface = NULL;

    HRESULT hr = gpSceneRenderTarget->GetSurfaceLevel(0, &pSceneSurface);

    if (SUCCEEDED(hr))
    {
        gpD3DDevice->SetRenderTarget(0, pSceneSurface);
        pSceneSurface->Release();
        pSceneSurface = NULL;
    }

    // clears the shadow map from the last frame
    gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0xFFFFFFFF, 1.0f, 0);

    // Vectors
    gpEnvironmentMappingShader->SetVector("gLightColor", &gLightColor);
    gpEnvironmentMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
    gpEnvironmentMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);
    
    // Matrices
    gpEnvironmentMappingShader->SetMatrix("gWorldMatrix", &matWorld);
    gpEnvironmentMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);
    
    // Textures
    gpEnvironmentMappingShader->SetTexture("DiffuseMap_Tex", gpTeapotDM);
    gpEnvironmentMappingShader->SetTexture("NormalMap_Tex", gpTeapotNM);
    gpEnvironmentMappingShader->SetTexture("SpecularMap_Tex", gpTeapotSM);
    gpEnvironmentMappingShader->SetTexture("EnvironmentMap_Tex", gpSnowENV);

    UINT numPasses = 0;
    gpEnvironmentMappingShader->Begin(&numPasses, NULL);

    for (UINT i = 0; i < numPasses; ++i)
    {
        gpEnvironmentMappingShader->BeginPass(i);
        gpTeapot->DrawSubset(0);
        gpEnvironmentMappingShader->EndPass();
    }

    gpEnvironmentMappingShader->End();

    // 2. apply post-processing

    // use hardware backbuffer
    gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
    pHWBackBuffer->Release();
    pHWBackBuffer = NULL;

    // post process effect to use 
    LPD3DXEFFECT effectToUse = gpNoEffect;

    if (gPostProcessIndex == 1)
    {
        effectToUse = gpGrayScale;
    }
    else if (gPostProcessIndex == 2)
    {
        effectToUse = gpSepia;
    }

    numPasses = 0;
    effectToUse->SetTexture("SceneTexture_Tex", gpSceneRenderTarget);
    effectToUse->Begin(&numPasses, NULL);
    {
        for (UINT i = 0; i < numPasses; ++i)
        {
            effectToUse->BeginPass(i);
            {
                // draw a fullscreen quad
                gpD3DDevice->SetStreamSource(0, gpFullscreenQuadVB, 0, sizeof(float) * 5);
                gpD3DDevice->SetIndices(gpFullscreenQuadIB);
                gpD3DDevice->SetVertexDeclaration(gpFullscreenQuadDecl);
                gpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);
            }
            effectToUse->EndPass();
        }
    }
    effectToUse->End();

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
    gpFont->DrawText(NULL, "Demo Framework\n\nESC: Quit demo\n1: Color \n2: Black and White\n3: Sepia", -1, &rct, 0, fontColor);
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

    // create a fullscreen quad
    InitFullScreenQuad();

    // create a render target
    HRESULT hr = gpD3DDevice->CreateTexture(WIN_WIDTH, WIN_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gpSceneRenderTarget, NULL);

    if (FAILED(hr))
    {
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
    // cubemap
    D3DXCreateCubeTextureFromFile(gpD3DDevice, "Snow_ENV.dds", &gpSnowENV);

    if (!gpSnowENV)
    {
        return false;
    }

    // shader loading
    gpEnvironmentMappingShader = LoadShader("EnvironmentMapping.fx");

    if (!gpEnvironmentMappingShader)
    {
        return false;
    }
    
    gpNoEffect = LoadShader("NoEffect.fx");

    if (!gpNoEffect)
    {
        return false;
    }

    gpGrayScale = LoadShader("Grayscale.fx");

    if (!gpGrayScale)
    {
        return false;
    }

    gpSepia = LoadShader("Sepia.fx");

    if (!gpSepia)
    {
        return false;
    }

    // textures
    gpTeapotDM = LoadTexture("Fieldstone_DM.tga");
    
    if (!gpTeapotDM)
    {
        return false;
    }
    
    gpTeapotNM = LoadTexture("Fieldstone_NM.tga");

    if (!gpTeapotNM)
    {
        return false;
    }

    gpTeapotSM = LoadTexture("Fieldstone_SM.tga");

    if (!gpTeapotSM)
    {
        return false;
    }

    // model loading
    gpTeapot = LoadModel("TeapotWithTangent.x");

    if (!gpTeapot)
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
// fullscreen quad
//------------------------------------------------------------
void InitFullScreenQuad()
{
    D3DVERTEXELEMENT9 vtxDesc[3];
    int offset = 0;
    int i = 0;

    // position
    vtxDesc[i].Stream = 0;
    vtxDesc[i].Offset = offset;
    vtxDesc[i].Type = D3DDECLTYPE_FLOAT3;
    vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT;
    vtxDesc[i].Usage = D3DDECLUSAGE_POSITION;
    vtxDesc[i].UsageIndex = 0;

    offset += sizeof(float) * 3;
    ++i;

    // UV coords 0 
    vtxDesc[i].Stream = 0;
    vtxDesc[i].Offset = offset;
    vtxDesc[i].Type = D3DDECLTYPE_FLOAT2;
    vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT;
    vtxDesc[i].Usage = D3DDECLUSAGE_TEXCOORD;
    vtxDesc[i].UsageIndex = 0;
    
    offset += sizeof(float) * 2;
    ++i;

    // end of the vertex format (D3DDECL_END())
    vtxDesc[i].Stream = 0xFF;
    vtxDesc[i].Offset = 0;
    vtxDesc[i].Type = D3DDECLTYPE_UNUSED;
    vtxDesc[i].Method = 0;
    vtxDesc[i].Usage = 0;
    vtxDesc[i].UsageIndex = 0;

    gpD3DDevice->CreateVertexDeclaration(vtxDesc, &gpFullscreenQuadDecl);

    gpD3DDevice->CreateVertexBuffer(offset * 4, 0, 0, D3DPOOL_MANAGED, &gpFullscreenQuadVB, NULL);

    void * vertexData = NULL;
    gpFullscreenQuadVB->Lock(0, 0, &vertexData, 0);
    {
        float* data = (float*)vertexData;
        // Pos0
        *data++ = -1.0f;
        *data++ = 1.0f;
        *data++ = 0.0f;

        // UV0
        *data++ = 0.0f;
        *data++ = 0.0f;

        // Pos1
        *data++ = 1.0f;
        *data++ = 1.0f;
        *data++ = 0.0f;

        // UV1
        *data++ = 1.0f;
        *data++ = 0.0f;

        // Pos2
        *data++ = 1.0f;
        *data++ = -1.0f;
        *data++ = 0.0f;

        // UV2
        *data++ = 1.0f;
        *data++ = 1.0f;

        // Pos3
        *data++ = -1.0f;
        *data++ = -1.0f;
        *data++ = 0.0f;

        // UV3 
        *data++ = 0.0f;
        *data++ = 1.0f;
    } 
    gpFullscreenQuadVB->Unlock();

    // create an index buffer
    gpD3DDevice->CreateIndexBuffer(sizeof(short) * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &gpFullscreenQuadIB, NULL);

    void * indexData = NULL;
    gpFullscreenQuadIB->Lock(0, 0, &indexData, 0);
    {
        unsigned short * data = (unsigned short*)indexData;
        *data++ = 0;
        *data++ = 1;
        *data++ = 3;
        *data++ = 3;
        *data++ = 1;
        *data++ = 2;
    }
    gpFullscreenQuadIB->Unlock();
}

//------------------------------------------------------------
// cleanup code
//------------------------------------------------------------

void Cleanup()
{
    if (gpFullscreenQuadDecl)
    {
        gpFullscreenQuadDecl->Release();
        gpFullscreenQuadDecl = NULL;
    }
    
    if (gpFullscreenQuadVB)
    {
        gpFullscreenQuadVB->Release();
        gpFullscreenQuadVB = NULL;
    }

    if (gpFullscreenQuadIB)
    {
        gpFullscreenQuadIB->Release();
        gpFullscreenQuadIB = NULL;
    }

    // release fonts
    if (gpFont)
    {
        gpFont->Release();
        gpFont = NULL;
    }

    // release models
    if (gpTeapot)
    {
        gpTeapot->Release();
        gpTeapot = NULL;
    }

    // release shaders
    if (gpEnvironmentMappingShader)
    {
        gpEnvironmentMappingShader->Release();
        gpEnvironmentMappingShader = NULL;
    }

    if (gpNoEffect)
    {
        gpNoEffect->Release();
        gpNoEffect = NULL;
    }


    // Release textures
    if (gpGrayScale)
    {
        gpGrayScale->Release();
        gpGrayScale = NULL;
    }

    if (gpSepia)
    {
        gpSepia->Release();
        gpSepia = NULL;
    }

    if (gpSceneRenderTarget)
    {
        gpSceneRenderTarget->Release();
        gpSceneRenderTarget = NULL;
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

