#pragma once
//infraestructura grafica
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
class CDXManager
{
public:
	CDXManager();
	~CDXManager();
	// Enumera todos los adapatadores gr�ficos habilitados y con controlador v�lido. Retorna el adaptador seleccionado por el usuario.
	static IDXGIAdapter* EnumAndChooseAdapter(HWND hWndOwner);
};

