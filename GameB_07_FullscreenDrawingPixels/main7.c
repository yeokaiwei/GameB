//#7 Visual Code needs another compiler and is slower than Visual Studio. VS Code is based on the Electron framework. Electron is a framework for creating native applications with web technologies. https://mspoweruser.com/xbox-pc-app-gains-huge-performance-gains-after-abandoning-electron-framework/
//#7 Linker -> Manifest File -> UAC Execution Level -> /asAdministrator will allow to force the User to have admin rights. Manifest files are XML files embedded in the .exe.
//#7 Virtual screens. https://learn.microsoft.com/en-us/windows/win32/gdi/the-virtual-screen

#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include <stdint.h> 

#include "main7.h"

HWND gh_GameWindow;

BOOL gb_GameIsRunning;

GAMEBITMAP g_BackBuffer;

RECT g_GameWindowSize;

MONITORINFO g_MonitorInfo = { sizeof(MONITORINFO) };

//#7 Standard int data types
int32_t g_MonitorWidth;
int32_t g_MonitorHeight;


int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow)
{

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(NULL, "Another instance of the program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    if (ERROR_SUCCESS != CreateMainGameWindow())
    {
        goto EXIT;
    }

    g_BackBuffer.bitMapInfo.bmiHeader.biSize = sizeof(g_BackBuffer.bitMapInfo.bmiHeader);
    g_BackBuffer.bitMapInfo.bmiHeader.biWidth = GAME_RESOLUTION_WIDTH;
    g_BackBuffer.bitMapInfo.bmiHeader.biHeight = GAME_RESOLUTION_HEIGHT;
    g_BackBuffer.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    g_BackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    g_BackBuffer.bitMapInfo.bmiHeader.biPlanes = 1;
    g_BackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == g_BackBuffer.Memory)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    memset(g_BackBuffer.Memory, 0x7F, GAME_DRAWINGAREA_MEMORYSIZE);


    MSG message = { 0 };

    gb_GameIsRunning = TRUE;

    while (TRUE == gb_GameIsRunning)
    {

        while (PeekMessageA(&message, gh_GameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        Sleep(1);
    }


EXIT:
    return (0);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT lResult = { 0 };

    switch (uMessage)
    {
    case WM_CLOSE:
    {
        gb_GameIsRunning = FALSE;
        PostQuitMessage(0);
        break;
    }

    default:
    {
        lResult = DefWindowProcA(hWindow, uMessage, wParam, lParam);
    }
    }


    return lResult;
}

DWORD CreateMainGameWindow(void)
{
    DWORD dw_Result = ERROR_SUCCESS;

    WNDCLASSEXA windowClass = { 0 };

    windowClass.cbSize = sizeof(WNDCLASSEX);

    windowClass.style = 0;

    windowClass.lpfnWndProc = MainWindowProc;

    windowClass.cbClsExtra = 0;

    windowClass.cbWndExtra = 0;

    windowClass.hInstance = GetModuleHandleA(NULL);

    windowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

    windowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    windowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);

    windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));

    windowClass.lpszMenuName = NULL;

    windowClass.lpszClassName = GAME_NAME"_WINDOWCLASS";

    //#7 Compatibility issue. SetProcessDPIAwareContext(...) will ensure GetMonitorA(...) will return the correct primary monitor resolution, not the scaled monitor size and resolution.
    //https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setprocessdpiaware. 
    //SetProcessDpiAwarenessContext(), uses the latest technologies and only works with Windows 10. However, it prevents the .exe from even running on Windows 7 and below. It is preferable to use Properties -> Manifest Tools -> Input Output -> Per Monitor High DPI Aware
    //SetProcessDPIAwareContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    if (RegisterClassExA(&windowClass) == 0)
    {
        dw_Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    gh_GameWindow = CreateWindowExA(WS_EX_CLIENTEDGE,
        GAME_NAME"_WINDOWCLASS",
        "Window Title",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        NULL, NULL, windowClass.hInstance, NULL);

    if (NULL == gh_GameWindow)
    {
        dw_Result = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    //#7 GetMonitorInfoA(...) will return the wrong value because it's not aware of the user's monitor position and DPI
    if (0 == GetMonitorInfoA(MonitorFromWindow(gh_GameWindow, MONITOR_DEFAULTTOPRIMARY), &g_MonitorInfo))
    {
        dw_Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto EXIT;
    }

    //#7 Beware of zoom level, DPI, virtual screen positiion. Consider what happens when users adjust their zoom level.
    g_MonitorWidth = g_MonitorInfo.rcMonitor.right - g_MonitorInfo.rcMonitor.left;
    g_MonitorHeight = g_MonitorInfo.rcMonitor.bottom - g_MonitorInfo.rcMonitor.top;

    //#7 Sets the window attributes e.g. size, has titlebar, has X, minimize, locked, etc
    //https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptra
    //Used to be SetWindowLongA(); From 2020 onwards, all Window builds will be x64. GWL_STYLE is GetWindowLong_Style
    if (0 == SetWindowLongPtrA(gh_GameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW))
    {
        dw_Result = GetLastError();

        goto EXIT;
   }

    //#7 https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if(SetWindowPos(gh_GameWindow, HWND_TOPMOST, g_MonitorInfo.rcMonitor.left, g_MonitorInfo.rcMonitor.top, g_MonitorWidth, g_MonitorHeight, SWP_NOOWNERZORDER| SWP_FRAMECHANGED))
    {
        dw_Result = GetLastError();

        goto EXIT;
    }

EXIT:
    return (dw_Result);
}

BOOL GameIsAlreadyRunning(void)
{
    HANDLE mutex = NULL;

    mutex = CreateMutexA(NULL, FALSE, GAME_NAME"_GameMutex");


    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }

}

void ProcessPlayerInput()
{
    //#7 short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    //#7 if (TRUE == EscapeKeyIsDown), pressing ESC won't work. TRUE looks only for 1.
    if (EscapeKeyIsDown) //C evaluates anything other than 0 as true.
    {
        SendMessageA(gh_GameWindow, WM_CLOSE, 0, 0);
    }

}

void RenderFrameGraphics(void)
{
    //#7 
    //memset(g_BackBuffer.Memory, 0xFF, 4); Sets 1 pixel in the graybox
    //memset(g_BackBuffer.Memory, 0xFF, (GAME_RESOLUTION_WIDTH*4)*4); //Draw 4 white lines at the bottom
    //memset(g_BackBuffer.Memory, 0xFF, GAME_DRAWINGAREA_MEMORYSIZE + 1);//Crash
    //memset(g_BackBuffer.Memory, 0xFF, GAME_DRAWINGAREA_MEMORYSIZE);
    
    PIXEL32 Pixel = { 0 };

    Pixel.Blue  = 0xff;
    Pixel.Green = 0;
    Pixel.Red   = 0;
    Pixel.Alpha = 0xff;

    //full blue screen
    for (int x = 0; x < (GAME_RESOLUTION_HEIGHT*GAME_RESOLUTION_WIDTH); x++)
    {
        memcpy((PIXEL32*)g_BackBuffer.Memory + x, &Pixel, 4);
    }

    HDC DeviceContext = GetDC(gh_GameWindow);

   //StretchDIBits(DeviceContext, 0, 0, 100, 100, 0, 0, 100, 100, g_BackBuffer.Memory, &g_BackBuffer.bitMapInfo, DIB_RGB_COLORS, SRCCOPY); //#7 no pink bits
    StretchDIBits(  DeviceContext, 
                    0, 0, 
                    g_MonitorWidth, g_MonitorHeight,
                    0 , 0, 
                    GAME_RESOLUTION_WIDTH, GAME_RESOLUTION_HEIGHT, 
                    g_BackBuffer.Memory, &g_BackBuffer.bitMapInfo, 
                    DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(gh_GameWindow, DeviceContext);
}