#ifndef D3DAPP_H
#define D3DAPP_H

#include <windows.h>
#include <string>
#include "D3DUtil.h"
#include "GameTimer.h"

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene();
	virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//鼠标事件
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	//计算FPS
	void CalculateFrameStats();

protected:
	HINSTANCE m_hAppInst;                            //实例句柄
	HWND m_hMainWnd;                                //主窗口句柄
	bool m_isAppPaused;
	bool m_isMinimized;
	bool m_isMaximized;
	bool m_isResizing;
	UINT m_4xMsaaQuality;                            //4重采样质量

	GameTimer m_timer;                                //计时器
	D3D_DRIVER_TYPE m_d3dDriverType;                //驱动类型
	ID3D11Device *m_pD3dDevice;                        //设备
	ID3D11DeviceContext *m_pImmediateContext;        //设备上下文
	IDXGISwapChain *m_pSwapChain;                    //交换链
	ID3D11Texture2D *m_pDepthStencilBuffer;            //深度模板缓冲
	ID3D11RenderTargetView *m_pRenderTargetView;    //渲染目标视图
	ID3D11DepthStencilView *m_pDepthStencilView;    //深度模板缓冲视图
	D3D11_VIEWPORT m_screenViewPort;                //视口

	std::wstring m_mainWndCaption;                    //窗口标题
	int m_clientWidth;
	int m_clientHeight;
	bool m_isEnable4xMsaa;                            //是否支持4重采样
};
#endif    //D3DAPP_H
