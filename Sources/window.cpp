#include "../Includes/window.h"

bool Window::InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR monitorHandle = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };
		GetMonitorInfo(monitorHandle, &monitorInfo);

		width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
		height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	}

	WNDCLASSEX wc =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WndProc,
		.cbClsExtra = NULL,
		.cbWndExtra = NULL,
		.hInstance = hInstance,
		.hIcon = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName = NULL,
		.lpszClassName = windowName,
		.hIconSm = LoadIcon(NULL, IDI_APPLICATION)
	};

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Cannot Register Window", L"Error", MB_OK);
		return false;
	}

	hwnd = CreateWindowEx(
		NULL,
		windowName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd)
	{
		MessageBox(0, L"Cannot Create Window", L"Error", MB_OK);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
