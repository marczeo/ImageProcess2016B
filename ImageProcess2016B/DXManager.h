#pragma once
//infraestructura grafica
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#ifndef SAFE_REALASE
	#define SAFE_RELEASE(X)if((X)){ (X)->Release(); (X)=0;}
#endif

class CDXManager
{
protected:
	ID3D11Device* m_pDevice; //Factory of Device Resources, es como una fachada de comandos
	ID3D11DeviceContext* m_pContext; //Command Queue and command Record and Execution
	IDXGISwapChain* m_pSwapChain; // Buffers Chain (Backbuffer)
public:
	CDXManager();
	~CDXManager();
	// Enumera todos los adapatadores gráficos habilitados y con controlador válido. Retorna el adaptador seleccionado por el usuario.
	static IDXGIAdapter* EnumAndChooseAdapter(HWND hWndOwner);
	//Inicializa los recursos de dispositivo, cadena de intercambio y contexto para enviar comandos al GPU o CPU
	bool Initialize(HWND hWnd, bool bUseWARO, IDXGIAdapter* pAdapter);
	//Terminar la sesión de DirectX11
	void Uninitialize();
};

