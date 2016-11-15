#include "stdafx.h"
#include "DXManager.h"
#include "IPImage.h"


CDXManager::CDXManager()
{
}


CDXManager::~CDXManager()
{
}


// Enumera todos los adapatadores gráficos habilitados y con controlador válido. Retorna el adaptador seleccionado por el usuario.
IDXGIAdapter* CDXManager::EnumAndChooseAdapter(HWND hWndOwner)
{
	IDXGIFactory* pFactory = NULL;
	//Solicitando una sola interfaz
	//Menor que cero error, mayor igual que cero suceso
	HRESULT hr = CreateDXGIFactory(IID_IDXGIFactory, (void**)&pFactory);
	if (FAILED(hr))
		return NULL;
	IDXGIAdapter* pAdapter = NULL;
	for (unsigned int iAdapter = 0;1;iAdapter++) 
	{
		hr = pFactory->EnumAdapters(iAdapter, &pAdapter);
		if (FAILED(hr))
			break;
		DXGI_ADAPTER_DESC dad;
		memset(&dad, 0, sizeof(dad));
		pAdapter->GetDesc(&dad);
		wchar_t szMessage[1024];
		wsprintf(szMessage,L"¿Desea utilizar éste adaptador?\r\nDescripción:%s\r\nMemoria dedicada de vídeo:\t%dMB\r\nMemoria Dedicada de Sistema:\t%dMB\r\nMemoria Compartida de Sistema:%dMB",
			dad.Description,dad.DedicatedVideoMemory/(1024*1024),
			dad.DedicatedSystemMemory/(1024*1024),
			dad.SharedSystemMemory/(1024*1024));
		switch (MessageBox(hWndOwner, szMessage, L"Selección de adaptador", MB_YESNOCANCEL | MB_ICONQUESTION))
		{
		case IDYES:
			pFactory->Release();
			pFactory = NULL;
			return pAdapter;
		case IDNO:
			//Liberar adaptador
			pAdapter->Release();
			pAdapter = NULL;
			break;
		case IDCANCEL:
			pAdapter->Release();
			pFactory->Release();
			return NULL;
		}
	}
	pFactory->Release();
	return pAdapter;
}


// Inicializa los recursos de dispositivo, cadena de intercambio y contexto para enviar comandos al GPU o CPU
bool CDXManager::Initialize(HWND hWnd, bool bUseWARP, IDXGIAdapter* pAdapter)
{
	//Tramites de cadena de intercambio
	DXGI_SWAP_CHAIN_DESC dscd;
	memset(&dscd, 0, sizeof(dscd));
	dscd.BufferCount = 2;
	//B8G8R8A8
	dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//59.98Hz de televisiones, numerador de 5998, denominador en 100 | si se deja en 0 es automatico 
	dscd.BufferDesc.RefreshRate.Numerator = 0;
	dscd.BufferDesc.RefreshRate.Denominator = 0;
	dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	//Transmitiendo informacion en orden progresivo (alta calidad) un cuadro a la vez
	dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	RECT rc;
	GetClientRect(hWnd, &rc);
	dscd.BufferDesc.Height = rc.bottom;
	dscd.BufferDesc.Width = rc.right;
	// - | Memoria de procesamiento de libre acceso
	dscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	dscd.Flags = 0;
	//Puerto de vision
	dscd.OutputWindow = hWnd;
	//Antialeasing aumentar cantidad de pixeles de entrada por uno de entrada
	dscd.SampleDesc.Count = 1;
	//Algoritmo de entrada
	dscd.SampleDesc.Quality = 0;
	//Nadie garantiza que primary sobreviva despues del intecambio
	dscd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dscd.Windowed = true;

	D3D_FEATURE_LEVEL Requested = D3D_FEATURE_LEVEL_11_0, Detected;

	if (bUseWARP)
	{
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, &Requested, 1, D3D11_SDK_VERSION, &dscd, &m_pSwapChain, &m_pDevice, &Detected, &m_pContext);
	}
	else
	{
		if (pAdapter)
		{
			HRESULT hr = D3D11CreateDeviceAndSwapChain(
				pAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0,
				&Requested, 1, D3D11_SDK_VERSION,
				&dscd, &m_pSwapChain, &m_pDevice, &Detected, &m_pContext);
			if (FAILED(hr)) return false;
		}
		else
		{
			HRESULT hr = D3D11CreateDeviceAndSwapChain(
				NULL, D3D_DRIVER_TYPE_HARDWARE, 0, 0,
				&Requested, 1, D3D11_SDK_VERSION,
				&dscd, &m_pSwapChain, &m_pDevice, &Detected, &m_pContext);
			if (FAILED(hr)) return false;
		}
	}
	return true;
}


// Terminar la sesión de Directx11
void CDXManager::Uninitialize()
{
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pSwapChain);
}

#include <d3dcompiler.h>
ID3D11ComputeShader * CDXManager::CompileCS(wchar_t * pszFilename, char * pszEntryPoint)
{
	//Objeto binario muy grande (BynaryLargeOBject)
	ID3DBlob* pDXIL=nullptr;	//DirectX Intermediate Languaje (puro binario, shader compilados) Se le pasa al controlador, y lo convierte en codigo nativo
	ID3DBlob* pErrors=nullptr;	//Errores y warnings de compilación
	ID3D11ComputeShader* pCS = nullptr;

	HRESULT hr = D3DCompileFromFile(pszFilename, 
		NULL, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE, 
		pszEntryPoint, "cs_5_0", 
		
	//DEPURACION
#ifndef _DEBUG
		D3DCOMPILE_OPTIMIZATION_LEVEL3|
#else
	D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG |
#endif
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &pDXIL, &pErrors);
	//FIN DEPURACION
	if (pErrors)
	{
		MessageBoxA(NULL,(char*) pErrors->GetBufferPointer(), "Errors or warnings", MB_ICONEXCLAMATION);
		pErrors->Release();
	}
	if (pDXIL)
	{
		//Construir codigo nativo
		hr = m_pDevice->CreateComputeShader(pDXIL->GetBufferPointer(), pDXIL->GetBufferSize(), NULL, &pCS);
		pDXIL->Release();
	}
	if (FAILED(hr))
		MessageBoxA(NULL, "Unable to compile file.", "Error", MB_ICONERROR);
	
	return pCS;
}

//Crear una textura 2D a partir de una Imagen CPU->GPU
ID3D11Texture2D* CDXManager::CreateTexture(CIPImage* pImage) 
{
	//1. Crear andamio para subir informacion al GPU
	//(stage)Andamio: buffer de memoria al alcance de Application y Kernel
	D3D11_TEXTURE2D_DESC dtd;
	memset(&dtd, 0, sizeof(dtd));
	dtd.ArraySize = 1;
	dtd.BindFlags = 0;
	dtd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dtd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dtd.MipLevels = 1;
	dtd.Width = pImage->m_nSizeX;
	dtd.Height = pImage->m_nSizeY;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_STAGING;
	ID3D11Texture2D* pStage = nullptr;
	m_pDevice->CreateTexture2D(&dtd, NULL, &pStage);
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pContext->Map(pStage, 0, D3D11_MAP_WRITE,0,&ms);
	
	//multiplos de 16, 128bit anidado
	unsigned char* pDest = (unsigned char*)ms.pData;
	for (int j = 0; j < pImage->m_nSizeY; j++)
	{		
		//En nuestra imagen nos movemos de pitch en pitch
		memcpy(pDest, &(*pImage)(0, j), pImage->m_nPitch);
		pDest += ms.RowPitch;
	}


	m_pContext->Unmap(pStage, 0);
	ID3D11Texture2D* pTexture = nullptr;
	dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.CPUAccessFlags = 0;
	m_pDevice->CreateTexture2D(&dtd, NULL, &pTexture);
	//Orden para el GPU
	m_pContext->CopyResource(pTexture,pStage);
	pStage->Release();
	return pTexture;
}
//Crear una imagen a partir de una textura GPU->CPU
CIPImage* CDXManager::CreateImage(ID3D11Texture2D* pTexture2D)
{
	return nullptr;
}