#pragma comment(lib, "legacy_stdio_definitions.lib")
#include <windowsx.h>//�����������õĺ�͸��ַ���Ĺ���
#include <sstream>
#include "D3DApp.h"

namespace
{
	D3DApp *g_d3dApp = NULL;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_d3dApp->MsgProc(hWnd, msg, wParam, lParam);
}


D3DApp::D3DApp(HINSTANCE hInstance) :
	m_hAppInst(hInstance),
	m_mainWndCaption(L"D3D11 Application"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_clientHeight(600),
	m_clientWidth(800),
	m_isEnable4xMsaa(false),
	m_hMainWnd(NULL),
	m_isAppPaused(false),
	m_isMaximized(false),
	m_isMinimized(false),
	m_isResizing(false),
	m_4xMsaaQuality(0),

	m_pD3dDevice(NULL),
	m_pImmediateContext(NULL),
	m_pSwapChain(NULL),
	m_pRenderTargetView(NULL),
	m_pDepthStencilBuffer(NULL)
{
	ZeroMemory(&m_screenViewPort, sizeof(D3D11_VIEWPORT));
	g_d3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilBuffer);
	ReleaseCOM(m_pSwapChain);
	ReleaseCOM(m_pDepthStencilView);

	if (m_pImmediateContext)
		m_pImmediateContext->ClearState();

	ReleaseCOM(m_pImmediateContext);
	ReleaseCOM(m_pD3dDevice);
}

HINSTANCE D3DApp::AppInst() const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd() const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>(m_clientWidth / m_clientHeight);
}

int D3DApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	m_timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.Tick();
			if (!m_isAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_timer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return static_cast<int>(msg.wParam);
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}

void D3DApp::OnResize()
{
	assert(m_pImmediateContext);
	assert(m_pD3dDevice);
	assert(m_pSwapChain);

#pragma region ���� resource view
	// Resource view ���buff������Ϊtypeless����view����;���Ǳ������texture resource������ʱ���ͣ�һ���������ò�Ҫ��typeless�����ͣ����������ܣ�
	//release old views
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilView);
	ReleaseCOM(m_pDepthStencilBuffer);

	HR(m_pSwapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D *backBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));//��һ��������index of the back buff , ����the interface type of  the buff;�����������Ƿ���ֵ
	HR(m_pD3dDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView));//�ڶ�������a pointer to a D3D11_RENDER_TARGET_VIEW_DESC, which can be null when the resource is typeless;
	ReleaseCOM(backBuffer);

	//create depth/stencil buffer and its view ����Buff
	D3D11_TEXTURE2D_DESC depthStencilDecs;//To create a texture, we need to fill out a D3D11_TEXTURE2D_DESC structure describing the texture to create.
	depthStencilDecs.Width = m_clientWidth;
	depthStencilDecs.Height = m_clientHeight;
	depthStencilDecs.MipLevels = 1;//for a depth/stencil buffer, our texture obly needs one mipmap level.
	depthStencilDecs.ArraySize = 1;//The number of textures in a texture array, we only need one texture here;
	depthStencilDecs.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//�Ƿ�ʹ��4�ز���
	if (m_isEnable4xMsaa)
	{
		depthStencilDecs.SampleDesc.Count = 4;// The ���ز������� for the depth/stencil buff must match the settings used for the render target;
		depthStencilDecs.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDecs.SampleDesc.Count = 1;
		depthStencilDecs.SampleDesc.Quality = 0;
	}

	depthStencilDecs.Usage = D3D11_USAGE_DEFAULT;
	//D3D11_USAGE_DEFAULT����GPU read and write CPU not read or write; 
	//D3D11_USAGE_IMMUTABLE GPU read-only, CPU and GPU can only write once, CPU not read;
	//D3D11_USAGE_DYNAMIC GPU read CPU write; incurs a performance PENALTY;
	//D3D11_USAGE_STAGING CPU can read a copy of the  resouce;  performance PENALTY
	//����back buffʱ����Ҫָ��������;������Ĭ��Ϊ IMMUTABLE

	depthStencilDecs.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//Specifies where the resource will be bound to the pipeline ORed;Other: D3D11_BIND_RENDER_TARGET || D3D11_BIND_SHADER_RESOURCE

	depthStencilDecs.CPUAccessFlags = 0;
	//�����Usage��Ӧ D3D11_CPU_ACCESS_WRITE  D3D11_CPU_ACCESS_READ
	depthStencilDecs.MiscFlags = 0;

	HR(m_pD3dDevice->CreateTexture2D(&depthStencilDecs, 0, &m_pDepthStencilBuffer));
	//The 2nd parameter of CreateTexture2D is a pointer to initial data to fill the texture with;�����ʼ�����ݣ�

	HR(m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView));
	//The Same to CreateRenderTargetView , the 2nd Desc canbe null if the resource is not typeless;


	//���µ�render target view �� depth/stencil view������
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	//the first parameter is the number of render targets we are binding;
#pragma endregion 

	//����viewport
	m_screenViewPort.TopLeftX = 0;
	m_screenViewPort.TopLeftY = 0;
	m_screenViewPort.Width = m_clientWidth;
	m_screenViewPort.Height = m_clientHeight;
	m_screenViewPort.MaxDepth = 1.0f;
	m_screenViewPort.MinDepth = 0.0f;

	m_pImmediateContext->RSSetViewports(1, &m_screenViewPort);
	//The number of viewports to bind;
}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//��һ�����ڱ������ʧȥ����״̬
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_isAppPaused = true;
			m_timer.Stop();
		}
		else
		{
			m_isAppPaused = false;
			m_timer.Start();
		}
		return 0;

		//���û��ػ洰��ʱ
	case WM_SIZE:
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = HIWORD(lParam);
		if (m_pD3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)//������С��
			{
				m_isAppPaused = true;
				m_isMinimized = true;
				m_isMaximized = true;
			}
			else if (wParam == SIZE_MAXIMIZED)//�������
			{
				m_isAppPaused = false;
				m_isMinimized = false;
				m_isMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)//���ڴ�С�ı�Ȳ������Ҳ������С��
			{
				if (m_isMinimized)
				{
					m_isAppPaused = false;
					m_isMinimized = false;
					OnResize();
				}
				else if (m_isMaximized)
				{
					m_isAppPaused = false;
					m_isMaximized = false;
					OnResize();
				}
				//���û����ڸı䴰�ڴ�Сʱ������OnResize(),���ı���ɺ���
				//����
				else if (m_isResizing)
				{

				}
				else
				{
					OnResize();
				}
			}
		}
		return 0;

		//�û���ʼ��ק�ı䴰�ڴ�С
	case WM_ENTERSIZEMOVE:
		m_isAppPaused = true;
		m_isResizing = true;
		m_timer.Stop();
		return 0;

		//�û��ı䴰�ڴ�С���
	case WM_EXITSIZEMOVE:
		m_isAppPaused = false;
		m_isResizing = false;
		m_timer.Start();
		OnResize();
		return  0;
		//��������
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		//The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key.
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

		//��ֹ���ڱ��̫С
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = m_hAppInst;
	wc.lpfnWndProc = MainWndProc;
	wc.lpszClassName = L"D3DWndClassName";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return false;
	}

	RECT rect{ 0, 0, m_clientWidth, m_clientHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_mainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, NULL, NULL, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return 0;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}
///����device��context���������Կ��Ŀ�������
bool D3DApp::InitDirect3D()
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |=  D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,//Ĭ�ϵ�adapter, specifies the display adapter we want the create device to represent��null - the primary display adapter;
		m_d3dDriverType,//ָ���������ͣ�����ѡ�� D3D_DRIVER_TYPE_HARDWATRE��D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWATRE, D3D_DRIVER_TYPE_WARP
		0,//This is used for supplying a software driver;
		createDeviceFlags,//D3D11_CREATE_DEVICE_DEBUG, D3D11_CREATE_DEVICE_SINGLETHREADED��
		0, //[pFeatureLevels] An array of D3D_FEATURE_LEVEL elements, whose order indicates the order in which to test feature level support;
		0,// [FeatureLevels] The number of D3D_FEATURE_LEVELs in the array pFeatureLevels;
		D3D11_SDK_VERSION,//[SDKVersion]
		&m_pD3dDevice,//[ppDevice]����ֵ���������feature support, and ������Դ
		&featureLevel,//[pFeatureLevel]����ֵ��Returns the first supported feature level in the pFeatureLevels array
		&m_pImmediateContext//[ppImmediateContext]����ֵ������render states, bind resources to the graphic pipeline, and issue rendering commands;
	);
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return hr;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported!", 0, 0);
		return false;
	}

	//check 4x msaa quality support
	HR(m_pD3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	//��佻�������� ����backbuff
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;//BufferDesc ���������ǽ�������back buffer�����ԣ�������Ҫ��עwidth,heigh,pixel format��
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//note: ʹ��DXGI_FORMAT_R8G8B8A8_UNORM (8-bits red,green, blue, alpha) for the back buff����Ϊ�������ʾ������֧�ֳ���24-bit��ɫ
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//�Ƿ���4�ز���
	if (m_isEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;//The number of ���ز���
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//DXGI_USAGE_RENDER_TARGET_OUTPUT������ȾΪback buffer,
	sd.BufferCount = 1;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_DISCARD �Կ�����ѡ�����Ч����ʾ��ʽ��
	sd.Flags = 0;
	//----------Note: ���������ʱ�ı���ز��������ԣ���Ҫ�ؽ�������the swap chain;
	
	//����ͨ��IDXGIFactory���������� 
	IDXGIDevice *pDxgiDevice = 0;
	HR(m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice)));

	IDXGIAdapter *pDxgiAdapter = 0;
	HR(pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter)));

	IDXGIFactory *pDxgiFactory = 0;
	HR(pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory)));
	//note��ͨ��::CreateDXGIFactory()����������һ��IDXGIFactoryʵ����

	HR(pDxgiFactory->CreateSwapChain(m_pD3dDevice, &sd, &m_pSwapChain));
	
	
	ReleaseCOM(pDxgiDevice);
	ReleaseCOM(pDxgiAdapter);
	ReleaseCOM(pDxgiFactory);

	OnResize();

	return true;
	//Note��DXGI(DirectX Graphics Infrastructure) ��һ��ר�Ŵ���ͼ��ѧ��صĿ�(like ��������ö���Կ���switching between windowed and full-screen mode)
}


void D3DApp::CalculateFrameStats()
{
	//����fps��ÿһ֡����
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.f / fps;

		std::wostringstream outs;
		outs.precision(6);//��������ʾ6λ
		outs << m_mainWndCaption << L"    " << L"FPS:" << fps << L"    "
			<< L"Frame Time:" << mspf << L" (ms) ";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void D3DApp::DrawScene()
{

}