// ImageProcess2016B.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ImageProcess2016B.h"
#include "DXManager.h"
#include "IPImage.h"
#include <math.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
CDXManager g_Manager;							// DXGI and D3d11 Manager

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	//Para probar el metodo
	/*IDXGIAdapter* pAdapter = CDXManager::EnumAndChooseAdapter(NULL);
	if (pAdapter) pAdapter->Release();*/



    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_IMAGEPROCESS2016B, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IMAGEPROCESS2016B));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMAGEPROCESS2016B));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	//Brocha del sistema (impidiendo que windows pinte por mi
	wcex.hbrBackground = 0; // (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_IMAGEPROCESS2016B);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   IDXGIAdapter* pAdapter = CDXManager::EnumAndChooseAdapter(NULL); //Adaptador

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //LUGAR CORRECTO PARA CARGAR
   if (!g_Manager.Initialize(hWnd, false, pAdapter))
	   MessageBox(NULL, L"Error al iniciar Directx11", L"Error fatal al iniciar DirectX11", MB_ICONERROR);
   if (pAdapter != NULL) {
	   DXGI_ADAPTER_DESC dad;
	   pAdapter->GetDesc(&dad);
	   SetWindowText(hWnd, dad.Description);
   }
   else
   {
	   SetWindowText(hWnd, L"No se seleccionó");
   }
   SAFE_RELEASE(pAdapter);
   //Fin lugar correcto

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//PAINTSTRUCT ps;
	//HDC hdc;
	static float sTime = 0.0f;
	static CIPImage* sImgSource = 0;
    switch (message)
    {
	case WM_CHAR:
		switch (wParam)
		{
		case 'c':
		case 'C':
			if (sImgSource) CIPImage::DestroyImage(sImgSource);
			sImgSource = CIPImage::CaptureDesktop();
			InvalidateRect(hWnd, NULL, false);		
			break;
		}
	case WM_CREATE:
		SetTimer(hWnd, 1, 10, NULL);
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case 1:
			sTime += 0.1;
			InvalidateRect(hWnd, NULL, false);
			break;		
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
			{
				//Caputara
				if (sImgSource)
					sImgSource->Draw(0, 0, hdc);

				//480 progresivo
				//Ya tenemos lienzo donde pintar, acceso a pixeles
				/*CIPImage* pImage = CIPImage::CreateImage(640, 480, sizeof(CIPImage::PIXEL) * 640);
				for(int j=0; j<480;j++)
					for (int i = 0; i < 640; i++)
					{
						//Ruido blanco contiene todas las frecuencias (imagenes habidas y por haber)
						//En audio "seseo" completa NO armonia
						unsigned char c = (unsigned char)rand();
						//Tiempo es el desplazamiento
						CIPImage::PIXEL P = { 127 + 127 * cos(10*sTime + 5*i*3.141592 / 180), 
							127+127*sin(10*sTime+4*j*i*3.141592/180),0,0}; //{i%266, j%256, (i+j)%256,0 };

						
						int x = i - 320, y = j - 240;
						if(x*x+y*y <10000)
							(*pImage)(i, j) = P;
						else
						{
							//Pixel shader sencillo
							CIPImage::PIXEL R = { c, 127+127*cos(sTime+0.03*i),c,c };
							(*pImage)(i, j) = R;
						}
					}
				pImage->Draw(0, 0, hdc);

				CIPImage::DestroyImage(pImage);*/
			}
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
