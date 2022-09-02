#pragma once 

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <iostream>

#if _DEBUG
	#define CHECK_HRESULT(result) check_hresult(result, __FILE__, __LINE__)
#else
	#define CHECK_HRESULT(result)
#endif

const float aspect_ratio = 16.0f / 9.0f;
const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = WIDTH / aspect_ratio;

static void print_debug(const char* result, const char* file, int line)
{
	std::string strFile = file;
	std::string strLine = std::to_string(line);
	std::string strResult = result;
	std::string tempMSG = "[" + strFile + "] " + strLine + " " + strResult;
	std::wstring stemp = std::wstring(tempMSG.begin(), tempMSG.end());
	LPCWSTR msg = stemp.c_str();

	MessageBox(0, msg, L"Error", MB_ICONERROR | MB_OK);
}

static void check_hresult(HRESULT result, const char* file, int line)
{
	switch (result)
	{
	case S_OK:
		break;
	case S_FALSE:
		print_debug("FALSE", file, line);
		break;
	case E_ABORT:
		print_debug("ABORT", file, line);
		break;
	case E_FAIL:
		print_debug("FAIL", file, line);
		break;
	case E_NOINTERFACE:
		print_debug("NO INTERFACE", file, line);
		break;
	case E_NOTIMPL:
		print_debug("NOT IMPLEMENTED", file, line);
		break;
	case E_POINTER:
		print_debug("POINTER IS NOT VALID", file, line);
		break;
	case E_UNEXPECTED:
		print_debug("UNEXPECTED FAILURE", file, line);
		break;
	case E_ACCESSDENIED:
		print_debug("ACCESS DENIED", file, line);
		break;
	case E_HANDLE:
		print_debug("HANDLE IS NOT VALID", file, line);
		break;
	case E_INVALIDARG:
		print_debug("INVALID ARGUMENT", file, line);
		break;
	case E_OUTOFMEMORY:
		print_debug("OUT OF MEMORY", file, line);
		break;
	}
}