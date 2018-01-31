/************************************************************************
Directx11学习笔记【2】 将HelloWin封装成类
2016.01 by zhangbaochong
/************************************************************************/
#pragma once
#include <windows.h>

static bool isPushEsc = false;//是否按下Esc键

class MyWindow
{
public:
	MyWindow();
	~MyWindow();
public:
	HWND GetHandle();//返回窗口句柄
	bool Create(int &width, int &height);//创建窗口
	void Run();//处理消息循环
	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);//消息处理
private:
	HWND m_hwnd;
	HINSTANCE m_hinstance;
	LPCWSTR m_name;
};