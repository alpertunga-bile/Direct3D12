#include "Includes/HelloTriangle.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int ncmdShow)
{
	HelloTriangle triangleApp(hInstance, ncmdShow, WIDTH, HEIGHT, false);

	triangleApp.OnInit();

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
			triangleApp.OnUpdate();
			triangleApp.OnRender();
		}
	}

	triangleApp.OnDestroy();

	return 0;
}