/************************************************************************
Directx11学习笔记【2】 将HelloWin封装成类
2016.01 by zhangbaochong
/************************************************************************/
#include "MyWindow.h"

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	int width = 800, height = 600;
	MyWindow *window = new MyWindow;
	if (window->Create(width, height))
	{
		window->Run();
	}
	return 0;
}