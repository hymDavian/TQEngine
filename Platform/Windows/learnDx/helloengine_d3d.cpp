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


//ȫ������
IDXGISwapChain* g_pSwapchain = nullptr;//ָ�򽻻����ӿڵ�ָ��
ID3D11Device* g_pDev = nullptr;//ָ��Direct3D�豸�ӿڵ�ָ��
ID3D11DeviceContext* g_pDevcon = nullptr;//ָ��Direct3D�豸�����ĵ�ָ��

ID3D11RenderTargetView* g_pRTView = nullptr;

ID3D11InputLayout* g_pLayout = nullptr;//ָ�����벼�ֵ�ָ��
ID3D11VertexShader* g_pVS = nullptr;//������ɫ��ָ��
ID3D11PixelShader* g_pPS = nullptr;//ƬԪ��ɫ��ָ��

ID3D11Buffer* g_pVBuffer = nullptr;//���㻺��
//���㻺��ṹ��
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
//��������
void CreateRenderTarget()
{
	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;


	//��ȡָ���̨��������ָ��

	g_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//������ȾĿ����ͼ
	g_pDev->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTView);

	pBackBuffer->Release();

	//����ͼ
	g_pDevcon->OMSetRenderTargets(1, &g_pRTView, NULL);
}
//�����ӿ�
void SetViewPort()
{
	D3D11_VIEWPORT viewport;//��������ʼ����ͼ�ߴ����
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));//�������������ȫ����0
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	g_pDevcon->RSSetViewports(1, &viewport);
}
//��ʼ���ܵ�(���غ�׼����ɫ��)
void InitPipeline()
{
	//���ز�������������ɫ��
	ID3DBlob* VS, * PS;
	//�ַ�ǰ׺�� L ��ζ�Ž�����ַ�תΪ wchar_t ������Ϊ���ڴ���ַ�����Ҫ������ڴ����Ա�ʾ���ַ�
	D3DReadFileToBlob(L"copy.vso", &VS);
	D3DReadFileToBlob(L"copy.pso", &PS);

	//��������ɫ���ŵ���ɫ��������
	g_pDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVS);
	g_pDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPS);
	//������ɫ������
	g_pDevcon->VSSetShader(g_pVS, 0, 0);
	g_pDevcon->PSSetShader(g_pPS, 0, 0);
	//�������벼�ֶ���
	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	g_pDev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &g_pLayout);
	g_pDevcon->IASetInputLayout(g_pLayout);

	VS->Release();
	PS->Release();
}
//��ʼ����״
void InitGraphics()
{
	//�����涨��Ķ���ṹ�嶨��������
	VERTEX OurVertices[] = {
		{XMFLOAT3(0.0f,0.5f,0.0f),XMFLOAT4(1.0f,0.0f,0.0f,1.0f)},
		{XMFLOAT3(0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,1.0f,0.0f,1.0f)},
		{XMFLOAT3(-0.45f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}
	};
	//�������㻺��
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	//ȷ��������Ӧ��δ��ж�ȡ��д�롣 ����Ƶ����һ���ؼ�����
	//D3D11_USAGE_DYNAMIC����Ϊ GPUֻ����CPUֻд�� �����ڶ���ÿ��֡������ CPU ����һ�ε���Դ
	bd.Usage = D3D11_USAGE_DYNAMIC;
	//���建������Сʹ�����Ƕ���Ķ����С��3��
	bd.ByteWidth = sizeof(VERTEX) * 3;
	//ȷ������������ΰ󶨵��ܵ�
	//����ʹ�ý���������Ϊ���㻺�����󶨵�������򼯽׶Ρ�
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//ָ����Դ����� CPU �������͡�
	//��Դ�ǿ�ӳ��ģ��Ա� CPU ���Ը��������ݡ� ʹ�ô˱�־��������Դ��������Ϊ�ܵ�����������ұ���ʹ�ö�̬�����ʹ�ô�����Դ
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDev->CreateBuffer(&bd, NULL, &g_pVBuffer);//��������
	//�����������ݵ�������
	D3D11_MAPPED_SUBRESOURCE ms;
	//��ȡָ�� ����Դ�а��������ݵ�ָ�룬���ܾ��Ը�����Դ�� GPU ���ʡ�
	//ӳ�仺����
	g_pDevcon->Map(g_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//��OurVertices�������ݵ�ms
	memcpy(ms.pData, OurVertices, sizeof(VERTEX) * 3);
	//ʹָ����Դ��ָ��ʧЧ������������ GPU �Ը���Դ�ķ��ʡ�
	g_pDevcon->Unmap(g_pVBuffer, NULL);
}


// ��������������,׼��ͼ����Դ
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

// �ͷ�GPU��Դ
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

// WindowProc����ԭ��
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// �κ�Windows�������ڵ�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;     // ���ڵľ�����ɺ������
	WNDCLASSEX wc; // ����ṹ���洰�������Ϣ



	// ����������Թ�ʹ��
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// ��������Ϣ���ṹ
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	// wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = _T("WindowClass1");

	// ע��window��
	RegisterClassEx(&wc);

	// ��������ʹ�þ��
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

	// ��ʾ�������
	ShowWindow(hWnd, nCmdShow);

	// ������ѭ��

	MSG msg; // �ýṹ����Windows�¼���Ϣ
	// �ȴ������е���һ����Ϣ��������洢��'msg��
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		// ������ϢתΪָ����ʽ
		TranslateMessage(&msg);

		// ����Ϣ���͸�WindowProc
		DispatchMessage(&msg);
	}



	// ��WM_QUIT��Ϣ����һ���ַ��ص�Windows
	return msg.wParam;
}

// ���ǳ������Ҫ��Ϣ�������
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
	case WM_SIZE: // ���ڳߴ緢���仯
	{
		if (g_pSwapchain != nullptr)
		{
			DiscardGraphicsResources();
		}
		wasHandled = true;
	}
	break;
	case WM_DESTROY: // ���ر�ʱ
	{
		DiscardGraphicsResources();
		PostQuitMessage(0); // ��ȫ�ر�Ӧ�ó���
		wasHandled = true;
	}
	break;
	case WM_DISPLAYCHANGE: // ��ʾ���ֱ��ʷ����仯
	{
		InvalidateRect(hWnd, nullptr, false); // ֪ͨϵͳ���ڵĿͻ�������Ҫ���»���
		wasHandled = true;
	}
	break;
	}

	// ����switch���û�д�����κ���Ϣ
	// return DefWindowProc(hWnd, message, wParam, lParam);
	if (!wasHandled)
	{
		result = DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}