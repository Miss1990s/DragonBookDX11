/************************************************************************
Directx11学习笔记【2】 将HelloWin封装成类
2016.01 by zhangbaochong
/************************************************************************/

#include "MyWindow.h"

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);//静态回调函数
static MyWindow *applicationHandle;//类的一个静态实例

MyWindow::MyWindow()
{
	isPushEsc = false;
	m_hwnd = NULL;
	m_name = L"HelloWin";
}


MyWindow::~MyWindow()
{
}


HWND MyWindow::GetHandle()
{
	return m_hwnd;
}

bool MyWindow::Create(int &width, int &height)
{
	WNDCLASSEX wnd;
	applicationHandle = this;
	m_hinstance = GetModuleHandle(NULL);
	wnd.cbClsExtra = 0;
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.cbWndExtra = 0;
	wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hIconSm = wnd.hIcon;
	wnd.hInstance = m_hinstance;
	wnd.lpfnWndProc = WndProc;
	wnd.lpszClassName = m_name;
	wnd.lpszMenuName = m_name;
	wnd.style = CS_VREDRAW | CS_HREDRAW;

	//注册窗口
	if (!RegisterClassEx(&wnd))
	{
		MessageBox(NULL, L"注册窗口失败", L"error", 0);
		return false;
	}
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_name, m_name, WS_OVERLAPPEDWINDOW, 0, 0, width, height,
		NULL, NULL, m_hinstance, NULL);
	//显示窗口设置其为焦点
	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);
	return true;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		//其他消息发送MessageHandler处理
	default:
		return applicationHandle->MessageHandler(hwnd, message, wparam, lparam);
	}
}

LRESULT CALLBACK MyWindow::MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		//检测按键消息
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)//用户按下退出键
			isPushEsc = true;
		return 0;

		//其他消息发送windows缺省处理
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
}

void MyWindow::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	bool isRuning = true;//控制是否退出消息循环
	while (isRuning)
	{
		//处理windows消息
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			isRuning = false;
		}
		else//按下esc键也退出
		{
			isRuning = !isPushEsc;

			//渲染等处理可以放在这儿
		}

	}
}