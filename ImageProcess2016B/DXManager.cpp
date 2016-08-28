#include "stdafx.h"
#include "DXManager.h"


CDXManager::CDXManager()
{
}


CDXManager::~CDXManager()
{
}


// Enumera todos los adapatadores gr�ficos habilitados y con controlador v�lido. Retorna el adaptador seleccionado por el usuario.
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
		wsprintf(szMessage,L"�Desea utilizar �ste adaptador?\r\nDescripci�n:%s\r\nMemoria dedicada de v�deo:\t%dMB\r\nMemoria Dedicada de Sistema:\t%dMB\r\nMemoria Compartida de Sistema:%dMB",
			dad.Description,dad.DedicatedVideoMemory/(1024*1024),
			dad.DedicatedSystemMemory/(1024*1024),
			dad.SharedSystemMemory/(1024*1024));
		switch (MessageBox(hWndOwner, szMessage, L"Selecci�n de adaptador", MB_YESNOCANCEL | MB_ICONQUESTION))
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
