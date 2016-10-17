#include "stdafx.h"
#include "IPImage.h"

using namespace std;


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

#include <fstream>
//8bits paletizado mono, 8bits
CIPImage * CIPImage::CreateImageFromFile(char * pszFileName)
{
	//1. Leer el encabezado del archivo
	//2. Leer la información de imagen (atributos)
	//3. Cargar la paleta de colores si es que aplica
	//4. Cargar el mapa de bits y traducción a CIPImage

	//DIB: Device Independent Bitmap  (Bitmap independiente del dispositivo)
	fstream in;
	in.open(pszFileName, ios::in | ios::binary);

	if (!in.is_open()) return nullptr;
	//Leer primara seccion de datos
	//1. File Header
	BITMAPFILEHEADER bfh;//Formato libre de regalias por parte de microsoft
	memset(&bfh, 0, sizeof(bfh));
	in.read((char*)&bfh, sizeof(bfh));

	//Par de docuentos especiales, firma del documento, indican que va bien, primero B, luego M (mas significativo  M)
	//Leyó primeros 2bytes del archivo
	if ('MB' != bfh.bfType) return nullptr;

	//2. Bitmap Info Header
	BITMAPINFOHEADER bih = { 0 }; //Equivalente a memset
	in.read((char*)&bih, sizeof(bih));
	//El encabezado tiene su propio tamaño y se puede verificar
	if (sizeof(BITMAPINFOHEADER) != bih.biSize) return nullptr;

	//Ya es seguro crear la imagen (Ahora si se pueden asignar recursos)
	CIPImage* pImage = CreateImage(bih.biWidth, bih.biHeight, sizeof(PIXEL)*bih.biWidth);

	//Sea cual sea el formato, la longitud es la medida, mide una linea de tamaño en bytes, nunca en pixeles
	//De pixeles a bits
	int nRowLength = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32); //31 por es el peor sobrante, cada grupo es de 4bytes
	switch (bih.biBitCount)
	{ 
		//Y es positivo hacia arriba
		//1 bit por pixel
		//todas las lineas de rastreo, debe ser multiplo de 32 bits, no importa sin sobran bits, 
		//Ej. 2x2, faltan 2bytes, 
		//Paletizados
	case 1:
	{
		RGBQUAD Paleta[2];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 2;
		in.read((char*)Paleta, sizeof(RGBQUAD)*nColors);
		//4.-Cargar el mapa de bits y traducirlo a PIXEL
		unsigned char* pRow = (unsigned char*)malloc(nRowLength);

		for (int j = bih.biHeight - 1; j >= 0; j--)
		{
			in.read((char*)pRow, nRowLength);
			for (int i = 0; i < bih.biWidth / 8; i++)
			{
				for (int h = 0; h <= 7; h++)
				{
					RGBQUAD& Color = Paleta[((pRow[i] >> (7 - h))) & 0x01];
					PIXEL& P = (*pImage)(((i * 8) + h), j);

					P.r = Color.rgbRed;
					P.g = Color.rgbGreen;
					P.b = Color.rgbBlue;
					P.a = 0xff;
				}

				/*P = (*pImage)(i+nRowLength, j);
				P.r = Color.rgbRed;
				P.g = Color.rgbGreen;
				P.b = Color.rgbBlue;
				P.a = 0xff;*/
			}
		}
		free(pRow);
	}
		break;
	case 4:
	{
		//Formato de la paleta
		//Numero de colores 2^n
		RGBQUAD Paleta[16];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 16;
		in.read((char*)Paleta, sizeof(RGBQUAD)*nColors);
		//4. Cargar el mapa de bits y traducirlo
		//Indices van sin signo
		//Malloc no invoca a contructor, new sí, usar malloc en casos masivos
		unsigned char* pRow = (unsigned char*)malloc(nRowLength); //Leer linea por linea
																  //comenzar desde la ultima fila, de abajo hacia arriba
		for (int j = bih.biHeight - 1; j >= 0; j--)
		{
			//Se lee de menor a mayor, de izquierda a derecha
			in.read((char*)pRow, nRowLength); //lee bytes
			for (int i = 0; i < bih.biWidth; i++)
			{
				//Sacando por referencia, para no hacer copia directa
				RGBQUAD& Color = Paleta[(pRow[i] >> 4) & 0xF];
				PIXEL& P = (*pImage)(i * 2, j);
				P.r = Color.rgbRed;
				P.g = Color.rgbGreen;
				P.b = Color.rgbBlue;
				P.a = 0xff;
				RGBQUAD& Color2 = Paleta[(pRow[i]) & 0xF];
				PIXEL& P2 = (*pImage)(i * 2 + 1, j);
				P2.r = Color2.rgbRed;
				P2.g = Color2.rgbGreen;
				P2.b = Color2.rgbBlue;
				P2.a = 0xff;
				//Puntero a una funcion, para calcular el canal alfa en base a color r,g,b.
			}
		}
		free(pRow);
	}
		break;
	case 8:
		//3. Leer paleta
		{
		//Formato de la paleta
		//Numero de colores 2^n
		RGBQUAD Paleta[256];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 256;
		in.read((char*)Paleta, sizeof(RGBQUAD)*nColors);
		//4. Cargar el mapa de bits y traducirlo
		//Indices van sin signo
		//Malloc no invoca a contructor, new sí, usar malloc en casos masivos
		unsigned char* pRow = (unsigned char*)malloc(nRowLength); //Leer linea por linea
		//comenzar desde la ultima fila, de abajo hacia arriba
		for (int j = bih.biHeight - 1; j >= 0; j--)
		{
			//Se lee de menor a mayor, de izquierda a derecha
			in.read((char*)pRow, nRowLength); //lee bytes
			for (int i = 0; i < bih.biWidth; i++)
			{
				//Sacando por referencia, para no hacer copia directa
				RGBQUAD& Color = Paleta[pRow[i]];
				PIXEL& P = (*pImage)(i, j);
				P.r = Color.rgbRed;
				P.g = Color.rgbGreen;
				P.b = Color.rgbBlue;
				P.a = 0xff;
				//Puntero a una funcion, para calcular el canal alfa en base a color r,g,b.
			}
		}
		free(pRow);
		}
		break;
	//No paletizados
	case 24:
	{
		unsigned char* pRow = (unsigned char*)malloc(nRowLength);
		for (int j = bih.biHeight - 1; j >= 0; j--)
		{
			in.read((char*)pRow, nRowLength);
			for (int i = 0; i < bih.biWidth; i++)
			{
				//RGBQUAD& Color = Paleta[pRow[i*3]];
				PIXEL& P = (*pImage)(i, j);
				P.r = pRow[(i * 3) + 2];
				P.g = pRow[(i * 3) + 1];
				P.b = pRow[(i * 3)];
				P.a = 0xff;
			}
		}
		free(pRow);
	}
		break;
	case 32:
	{
		unsigned char* pRow = (unsigned char*)malloc(nRowLength);
		for (int j = bih.biHeight - 1; j >= 0; j--)
		{
			in.read((char*)pRow, nRowLength);
			for (int i = 0; i < bih.biWidth; i++)
			{
				//RGBQUAD& Color = Paleta[pRow[i*3]];
				PIXEL& P = (*pImage)(i, j);
				P.r = pRow[(i * 4)];
				P.g = pRow[(i * 4) + 2];
				P.b = pRow[(i * 4) + 1];
				P.a = 0xff;
			}
		}
		free(pRow);
	}
		break;

	}
	return pImage;

}