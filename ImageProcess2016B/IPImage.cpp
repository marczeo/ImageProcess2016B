#include "stdafx.h"
#include "IPImage.h"


CIPImage::CIPImage()
{
	m_pBuffer = NULL;
	m_nPitch = m_nSizeX = m_nSizeY = 0;
}


CIPImage::~CIPImage()
{
}

CIPImage::PIXEL& CIPImage::operator()(int i, int j)
{
	static PIXEL Dummy;
	if (i >= 0 && i < m_nSizeX && j>=0 && j < m_nSizeY)
		return *(PIXEL*)((char*)m_pBuffer + m_nPitch*j + i * sizeof(PIXEL));
	return Dummy;
}

CIPImage* CIPImage::CreateImage(int sx, int sy, int nPitch)
{
	CIPImage* pNewImage = new CIPImage();
	//Pitch tamaño horizontal de una linea bytes
	pNewImage->m_pBuffer = (PIXEL*)malloc(nPitch*sy);
	pNewImage->m_nSizeX = sx;
	pNewImage->m_nSizeY = sy;
	pNewImage->m_nPitch = nPitch;
	return pNewImage;
}

void CIPImage::DestroyImage(CIPImage* pImage)
{
	free(pImage->m_pBuffer);
	delete pImage;
}

//hdc device context donde será impreso
void CIPImage::Draw(int x, int y, HDC hdc)
{
	//Dibujar usando GDI Windows
	HDC hdcMem = CreateCompatibleDC(hdc);
	//Solicitar papel para imprimir (area de dibujo), se transfire pixeles de mi imagen a memoria administrada por windows en rafagaz
	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, m_nSizeX, m_nSizeY);

	SelectObject(hdcMem, hbmMem);
	SetBitmapBits(hbmMem, m_nPitch*m_nSizeY, m_pBuffer);
	BitBlt(hdc, x, y, m_nSizeX, m_nSizeY, hdcMem, 0, 0, SRCCOPY);

	DeleteObject(hbmMem);
	DeleteObject(hdcMem);
}
CIPImage* CIPImage::CaptureDesktop() 
{
	HDC hdcDesktop = CreateDC(L"DISPLAY", 0, 0, 0);
	HDC hdcMem = CreateCompatibleDC(hdcDesktop);
	int sx, sy;
	//Optener resolución
	sx = GetDeviceCaps(hdcDesktop, HORZRES);
	sy = GetDeviceCaps(hdcDesktop, VERTRES);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdcDesktop, sx, sy);
	SelectObject(hdcMem, hbmMem);
	BitBlt(hdcMem, 0, 0, sx, sy, hdcDesktop, 0, 0, SRCCOPY);
	CIPImage* pImage = CreateImage(sx, sy, sizeof(PIXEL)*sx);
	GetBitmapBits(hbmMem, pImage->m_nPitch*sy, pImage->m_pBuffer);
	DeleteDC(hdcMem);
	DeleteObject(hbmMem);
	DeleteDC(hdcDesktop);
	return pImage;

}