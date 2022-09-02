#pragma once

#include "common.h"

class Window
{
public:
	bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

	HWND GetHWND() { return hwnd; }

private:
	HWND hwnd = nullptr;
	LPCTSTR windowName = L"DirectX12";
	LPCTSTR windowTitle = L"DirectX12";
	bool fullscreen = false;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
