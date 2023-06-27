#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

//WindowProc函数原型
LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

//任何Windows程序的入口点
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
    HWND hWnd;//窗口的句柄，由函数填充
    WNDCLASSEX wc;//这个结构保存窗口类的信息

    //清除窗口类以供使用
    ZeroMemory(&wc,sizeof(WNDCLASSEX));

    //用所需信息填充结构
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = _T("WindowClass1");

    //注册window类
    RegisterClassEx(&wc);

    //创建窗口使用句柄
    hWnd = CreateWindowEx(0,
                        _T("WindowClass1"),//窗口名
                        _T("Hello,Engine!"),//标题
                        WS_OVERLAPPEDWINDOW,//窗口样式
                        300,//x坐标
                        300,//y坐标
                        500,//宽度
                        400,//高度
                        NULL,//父级窗口
                        NULL,//没有菜单
                        hInstance,//应用的句柄
                        NULL //多窗口
    );

    //显示这个窗口
    ShowWindow(hWnd,nCmdShow);

    //进入主循环

    MSG msg;//该结构包含Windows事件消息
    //等待队列中的下一条消息，将结果存储在'msg中
    while(GetMessage(&msg,NULL,0,0))
    {
        //按键消息转为指定格式
        TranslateMessage(&msg);

        //将消息发送给WindowProc
        DispatchMessage(&msg);
    }

    //将WM_QUIT消息的这一部分返回到Windows
    return msg.wParam;
}

//这是程序的主要消息处理程序
LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    //sort through and find what code to run for the message given
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd,&ps);
            RECT rec = {20,20,68,80};
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

            FillRect(hdc,&rec,brush);

            EndPaint(hWnd,&ps);
        }
        break;

        case WM_DESTROY://当关闭时
        {
            PostQuitMessage(0);//完全关闭应用程序
            return 0;
        }
        break;
    }

    //处理switch语句没有处理的任何消息
    return DefWindowProc(hWnd,message,wParam,lParam);
}