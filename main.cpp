#include "Includes/Application.h"
#include "Includes/window.h"

void windowsMain()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int ncmdShow)
{
	#ifdef _DEBUG
		AllocConsole();
	#endif

	Window window;

	if (!window.InitializeWindow(hInstance, ncmdShow, WIDTH, HEIGHT, false))
	{
		MessageBox(0, L"Cannot Initialize Window", L"Error", MB_OK);
		exit(1);
	}

	ev::Application app;

	windowsMain();

	return 0;
}