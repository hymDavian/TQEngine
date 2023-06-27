#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdint.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

const uint32_t SCREEN_WIDTH = 960;
const uint32_t SCREEN_HEIGHT = 480;


//全局声明
IDXGISwapChain* g_pSwapchain = nullptr;//指向交换链接口的指针
ID3D11Device* g_pDev = nullptr;//指向Direct3D设备接口的指针
ID3D11DeviceContext* g_pDevcon = nullptr;//指向Direct3D设备上下文的指针

ID3D11RenderTargetView* g_pRTView = nullptr;

ID3D11InputLayout* g_pLayout = nullptr;//指向输入布局的指针
ID3D11VertexShader* g_pVS = nullptr;//顶点着色器指针
ID3D11PixelShader* g_pPS = nullptr;//片元着色器指针

ID3D11Buffer* g_pVBuffer = nullptr;//顶点缓存
//顶点缓存结构体
struct VERTEX
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

template <class T>
inline void SafeRelease(T** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = nullptr;
	}
}
//创建画布
void CreateRenderTarget()
{
	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;


	//获取指向后台缓冲区的指针

	g_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//创建渲染目标视图
	g_pDev->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTView);

	pBackBuffer->Release();

	//绑定视图
	g_pDevcon->OMSetRenderTargets(1, &g_pRTView, NULL);
}
//设置视口
void SetViewPort()
{
	D3D11_VIEWPORT viewport;//声明并初始化视图尺寸对象
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));//将这个对象内容全部置0
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	g_pDevcon->RSSetViewports(1, &viewport);
}
//初始化管道(加载和准备着色器)
void InitPipeline()
{
	//加载并编译这两个着色器
	ID3DBlob* VS, * PS;
	//字符前缀加 L 意味着将这个字符转为 wchar_t 可以作为长内存的字符，需要更多的内存用以表示此字符
	D3DReadFileToBlob(L"copy.vso", &VS);
	D3DReadFileToBlob(L"copy.pso", &PS);

	//将两个着色器放到着色器对象内
	g_pDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVS);
	g_pDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPS);
	//设置着色器对象
	g_pDevcon->VSSetShader(g_pVS, 0, 0);
	g_pDevcon->PSSetShader(g_pPS, 0, 0);
	//创建输入布局对象
	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	g_pDev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &g_pLayout);
	g_pDevcon->IASetInputLayout(g_pLayout);

	VS->Release();
	PS->Release();
}
//初始化形状
void InitGraphics()
{
	//用上面定义的顶点结构体定义三角面
	VERTEX OurVertices[] = {
		{XMFLOAT3(0.0f,0.5f,0.0f),XMFLOAT4(1.0f,0.0f,0.0f,1.0f)},
		{XMFLOAT3(0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,1.0f,0.0f,1.0f)},
		{XMFLOAT3(-0.45f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}
	};
	//创建顶点缓存
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	//确定缓冲区应如何从中读取和写入。 更新频率是一个关键因素
	//D3D11_USAGE_DYNAMIC设置为 GPU只读，CPU只写， 适用于对于每个帧至少由 CPU 更新一次的资源
	bd.Usage = D3D11_USAGE_DYNAMIC;
	//定义缓冲区大小使用我们定义的顶点大小的3倍
	bd.ByteWidth = sizeof(VERTEX) * 3;
	//确定缓冲区将如何绑定到管道
	//这里使用将缓冲区作为顶点缓冲区绑定到输入程序集阶段。
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//指定资源允许的 CPU 访问类型。
	//资源是可映射的，以便 CPU 可以更改其内容。 使用此标志创建的资源不能设置为管道的输出，并且必须使用动态或过渡使用创建资源
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDev->CreateBuffer(&bd, NULL, &g_pVBuffer);//创建缓存
	//拷贝顶点数据到缓存区
	D3D11_MAPPED_SUBRESOURCE ms;
	//获取指向 子资源中包含的数据的指针，并拒绝对该子资源的 GPU 访问。
	//映射缓冲区
	g_pDevcon->Map(g_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//从OurVertices拷贝数据到ms
	memcpy(ms.pData, OurVertices, sizeof(VERTEX) * 3);
	//使指向资源的指针失效，并重新允许 GPU 对该资源的访问。
	g_pDevcon->Unmap(g_pVBuffer, NULL);
}


// 创建画布，画笔,准备图形资源
HRESULT CreateGraphicResources(HWND hWnd)
{
	HRESULT hr = S_OK; // long 0
	if (g_pSwapchain == nullptr)
	{
		// create a struct to hold information about the swap chain
		DXGI_SWAP_CHAIN_DESC scd;

		// clear out the struct for use
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		// fill the swap chain description struct
		scd.BufferCount = 1;                                    // one back buffer
		scd.BufferDesc.Width = SCREEN_WIDTH;
		scd.BufferDesc.Height = SCREEN_HEIGHT;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
		scd.OutputWindow = hWnd;                                // the window to be used
		scd.SampleDesc.Count = 4;                               // how many multisamples
		scd.Windowed = TRUE;                                    // windowed/full-screen mode
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

		const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1 };
		D3D_FEATURE_LEVEL FeatureLevelSupported;

		HRESULT hr = S_OK;

		// create a device, device context and swap chain using the information in the scd struct
		hr = D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			FeatureLevels,
			_countof(FeatureLevels),
			D3D11_SDK_VERSION,
			&scd,
			&g_pSwapchain,
			&g_pDev,
			&FeatureLevelSupported,
			&g_pDevcon);

		if (hr == E_INVALIDARG) {
			hr = D3D11CreateDeviceAndSwapChain(NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				0,
				&FeatureLevelSupported,
				1,
				D3D11_SDK_VERSION,
				&scd,
				&g_pSwapchain,
				&g_pDev,
				NULL,
				&g_pDevcon);

		}

		if (hr == S_OK) {
			CreateRenderTarget();
			SetViewPort();
			InitPipeline();
			InitGraphics();
		}
	}
	return hr;
}

// 释放GPU资源
void DiscardGraphicsResources()
{
	//SafeRelease(&pRenderTarget);
	//SafeRelease(&pLightSlateGrayBrush);
	//SafeRelease(&pCornflowerBlueBrush);
	SafeRelease(&g_pLayout);
	SafeRelease(&g_pVS);
	SafeRelease(&g_pPS);
	SafeRelease(&g_pVBuffer);
	SafeRelease(&g_pSwapchain);
	SafeRelease(&g_pRTView);
	SafeRelease(&g_pDev);
	SafeRelease(&g_pDevcon);
}

// this is the function used to render a single frame
void RenderFrame()
{
	// clear the back buffer to a deep blue
	const FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pDevcon->ClearRenderTargetView(g_pRTView, clearColor);

	// do 3D rendering on the back buffer here
	{
		// select which vertex buffer to display
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		g_pDevcon->IASetVertexBuffers(0, 1, &g_pVBuffer, &stride, &offset);

		// select which primtive type we are using
		g_pDevcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw the vertex buffer to the back buffer
		g_pDevcon->Draw(3, 0);
	}

	// swap the back buffer and the front buffer
	g_pSwapchain->Present(0, 0);
}

// WindowProc函数原型
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// 任何Windows程序的入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;     // 窗口的句柄，由函数填充
	WNDCLASSEX wc; // 这个结构保存窗口类的信息



	// 清除窗口类以供使用
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// 用所需信息填充结构
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	// wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = _T("WindowClass1");

	// 注册window类
	RegisterClassEx(&wc);

	// 创建窗口使用句柄
	hWnd = CreateWindowEx(0,
		_T("WindowClass1"),                   // name of the window class
		_T("Hello, Engine![Direct 3D]"),      // title of the window
		WS_OVERLAPPEDWINDOW,                  // window style
		100,                                  // x-position of the window
		100,                                  // y-position of the window
		SCREEN_WIDTH,                         // width of the window
		SCREEN_HEIGHT,                        // height of the window
		NULL,                                 // we have no parent window, NULL
		NULL,                                 // we aren't using menus, NULL
		hInstance,                            // application handle
		NULL                                // used with multiple windows, NULL
	);

	// 显示这个窗口
	ShowWindow(hWnd, nCmdShow);

	// 进入主循环

	MSG msg; // 该结构包含Windows事件消息
	// 等待队列中的下一条消息，将结果存储在'msg中
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		// 按键消息转为指定格式
		TranslateMessage(&msg);

		// 将消息发送给WindowProc
		DispatchMessage(&msg);
	}



	// 将WM_QUIT消息的这一部分返回到Windows
	return msg.wParam;
}

// 这是程序的主要消息处理程序
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	bool wasHandled = false;

	// sort through and find what code to run for the message given
	switch (message)
	{
	case WM_CREATE:
	{
		wasHandled = true;
	}
	break;

	case WM_PAINT:
	{
		result = CreateGraphicResources(hWnd);
		RenderFrame();
		wasHandled = true;
	}
	break;
	case WM_SIZE: // 窗口尺寸发生变化
	{
		if (g_pSwapchain != nullptr)
		{
			DiscardGraphicsResources();
		}
		wasHandled = true;
	}
	break;
	case WM_DESTROY: // 当关闭时
	{
		DiscardGraphicsResources();
		PostQuitMessage(0); // 完全关闭应用程序
		wasHandled = true;
	}
	break;
	case WM_DISPLAYCHANGE: // 显示器分辨率发生变化
	{
		InvalidateRect(hWnd, nullptr, false); // 通知系统窗口的客户区域需要重新绘制
		wasHandled = true;
	}
	break;
	}

	// 处理switch语句没有处理的任何消息
	// return DefWindowProc(hWnd, message, wParam, lParam);
	if (!wasHandled)
	{
		result = DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}