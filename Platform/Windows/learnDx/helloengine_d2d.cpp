#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <d2d1.h>

ID2D1Factory *pFactory = nullptr;
ID2D1HwndRenderTarget *pRenderTarget = nullptr;
ID2D1SolidColorBrush *pLightSlateGrayBrush = nullptr;
ID2D1SolidColorBrush *pCornflowerBlueBrush = nullptr;

template <class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}

// 创建画布，画笔
HRESULT CreateGraphicResources(HWND hWnd)
{
    HRESULT hr = S_OK; // long 0
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hWnd, size),
            &pRenderTarget);
        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray), &pLightSlateGrayBrush);
        }
        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &pCornflowerBlueBrush);
        }
    }
    return hr;
}

// 释放GPU资源
void DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pLightSlateGrayBrush);
    SafeRelease(&pCornflowerBlueBrush);
}

// WindowProc函数原型
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// 任何Windows程序的入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;     // 窗口的句柄，由函数填充
    WNDCLASSEX wc; // 这个结构保存窗口类的信息

    // 初始化com组件
    // CoInitializeEx
    // 初始化第一个参数固定为0,而参数类型需要为LPVOID 所以使用 nullptr
    // COINIT_APARTMENTTHREADED 表示以STA附加程序内方式运行，隐藏自身，并绘制信息通知到此程序
    // COINIT_DISABLE_OLE1DDE 关闭一些过时的com功能，以减少不必要的开销
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        return 1;
    }

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
                          _T("WindowClass1"), // 窗口名
                          // _T("Hello,Engine!"),//标题
                          _T("Hello,Engine!(Direct 2D)"),
                          WS_OVERLAPPEDWINDOW, // 窗口样式
                          // 300,//x坐标
                          // 300,//y坐标
                          // 500,//宽度
                          // 400,//高度
                          100,
                          100,
                          960,
                          540,
                          NULL,      // 父级窗口
                          NULL,      // 没有菜单
                          hInstance, // 应用的句柄
                          NULL       // 多窗口
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

    // 结束com组件
    CoUninitialize();

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
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            result = -1; // 创建窗口错误
            return result;
        }
        wasHandled = true;
        result = 0;
    }
    break;

    case WM_PAINT:
    {
        HRESULT hr = CreateGraphicResources(hWnd);
        if (SUCCEEDED(hr))
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            // 开始gpu绘制命令
            pRenderTarget->BeginDraw();
            // 清理背景白色
            pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
            // 检索绘制区域大小
            D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
            // 绘制一个背景格
            int width = static_cast<int>(rtSize.width);
            int height = static_cast<int>(rtSize.height);
            for (int x = 0; x < width; x += 10)
            {
                //(起点,终点,使用笔刷,宽度)
                pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<float>(x), 0.0f),
                    D2D1::Point2F(static_cast<float>(x), rtSize.height),
                    pLightSlateGrayBrush,
                    0.5f);
            }
            for (int y = 0; y < height; y += 10)
            {
                pRenderTarget->DrawLine(
                    D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                    D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                    pLightSlateGrayBrush,
                    0.5f);
            }

            // 画2个矩形
            D2D1_RECT_F rectangle1 = D2D1::RectF(
                rtSize.width / 2 - 50.0f,
                rtSize.height / 2 - 50.0f,
                rtSize.width / 2 + 50.0f,
                rtSize.height / 2 + 50.0f);
            D2D1_RECT_F rectangle2 = D2D1::RectF(
                rtSize.width / 2 - 100.0f,
                rtSize.height / 2 - 100.0f,
                rtSize.width / 2 + 100.0f,
                rtSize.height / 2 + 100.0f);

            // 填充矩形
            pRenderTarget->FillRectangle(&rectangle1, pLightSlateGrayBrush);
            // 绘制矩形轮廓
            pRenderTarget->DrawRectangle(&rectangle2, pCornflowerBlueBrush);

            // 结束gpu绘制命令
            hr = pRenderTarget->EndDraw();
            if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
            {
                DiscardGraphicsResources();
            }
            EndPaint(hWnd, &ps);
        }
        wasHandled = true;
    }
    break;
    case WM_SIZE: // 窗口尺寸发生变化
    {
        // 通知gpu调整画布大小，抛弃所有绘图资源，重新建立
        if (pRenderTarget != nullptr)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
            pRenderTarget->Resize(size);
        }
        wasHandled = true;
    }
    break;
    case WM_DESTROY: // 当关闭时
    {
        DiscardGraphicsResources();
        if (pFactory)
        {
            pFactory->Release();
            pFactory = nullptr;
        }
        PostQuitMessage(0); // 完全关闭应用程序
        result = 0;
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