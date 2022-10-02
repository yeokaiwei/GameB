//#6 Github. Gitignore tells Git which files to ignore. If you choose Visual Studio, it will upload .c, .cpp, .h files but ignore the things like bitmaps, encryption/private keys to web servers and backend databases, sound files, assets.

#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include <stdint.h> //#6 Includes standard integers.

#include "main6.h"

HWND gh_GameWindow;

BOOL gb_GameIsRunning;

//#6 gbm_DrawingSurface is the backbuffer
GAMEBITMAP g_BackBuffer;

//#6 For the width and height for fullscreen
RECT g_GameWindowSize;

//#6 CRITICAL. You must initialize the sizeof MONITORINFO, which is weird. No compiler warnings. Remember, initialize {}, not assign =.
MONITORINFO g_MonitorInfo = { sizeof(MONITORINFO) };

int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow)
{ //#5 HERE 

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(NULL, "Another instance of the program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    if ( ERROR_SUCCESS != CreateMainGameWindow() )
    {
        goto EXIT;
    }

    //#6 Setting the backbuffer and its colour from black 
    g_BackBuffer.bitMapInfo.bmiHeader.biSize = sizeof(g_BackBuffer.bitMapInfo.bmiHeader);
    g_BackBuffer.bitMapInfo.bmiHeader.biWidth = GAME_RESOLUTION_WIDTH;
    g_BackBuffer.bitMapInfo.bmiHeader.biHeight = GAME_RESOLUTION_HEIGHT;
    g_BackBuffer.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    g_BackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB; 
    g_BackBuffer.bitMapInfo.bmiHeader.biPlanes = 1; 
    g_BackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    //#6 To view g_BackBuffer.Memory in Memory. Debug-> Window-> Memory -> Memory 1. Easier to set columns to 16 and match the address from the Watch Window. e.g. At 0x000001ab97b00000, you can set the corresponsding 00 00 00 00 to BGRA. In memory, it's not RGBA, it's BGRA, not a typo.
    if (NULL == g_BackBuffer.Memory)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    //0xFF all provides WHITE. There is no method to set it to all blue, green,red. If you use 0x0000FFFF (BGRA), it will just take the last 2 FF to give us all white.
    memset(g_BackBuffer.Memory,0x7F,GAME_DRAWINGAREA_MEMORYSIZE);


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
    DWORD dwResult               = ERROR_SUCCESS;

    WNDCLASSEXA windowClass     = { 0 };

    windowClass.cbSize          = sizeof(WNDCLASSEX);

    windowClass.style           = 0;

    windowClass.lpfnWndProc     = MainWindowProc;

    windowClass.cbClsExtra      = 0;

    windowClass.cbWndExtra      = 0;

    windowClass.hInstance       = GetModuleHandleA(NULL);

    windowClass.hIcon           = LoadIconA(NULL, IDI_APPLICATION);

    windowClass.hIconSm         = LoadIconA(NULL, IDI_APPLICATION);

    windowClass.hCursor         = LoadCursorA(NULL, IDC_ARROW);

    //#6 windowClass.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
    //#define RGB ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(b))<<16)))
    //CORREF is a DWORD which is a double word which is 32-bit
    windowClass.hbrBackground   = CreateSolidBrush(RGB(255, 0, 255));

    windowClass.lpszMenuName    = NULL;

    windowClass.lpszClassName   = GAME_NAME"_WINDOWCLASS";

    if (RegisterClassExA(&windowClass) == 0)
    {
        dwResult = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    gh_GameWindow = CreateWindowExA(    WS_EX_CLIENTEDGE,
                                        GAME_NAME"_WINDOWCLASS",
                                        "Window Title",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        640, 480,
                                        NULL, NULL, windowClass.hInstance, NULL);

    if (NULL == gh_GameWindow)
    {
        dwResult = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    //#6 After window creation, GetMonitorInfoA( [in] HMONITOR hMonitor, [out] LPMONITORINFO lpmi)
    //MonitorFromWindow(gh_GameWindow, MONITOR_DEFAULTTOPRIMARY) gets a window handle and what to default to if you have no window 
    if ( 0 == GetMonitorInfoA(MonitorFromWindow(gh_GameWindow, MONITOR_DEFAULTTOPRIMARY), &g_MonitorInfo) )
    {
        //Documentation does not state to use GetLastErrror();
        dwResult = ERROR_MONITOR_NO_DESCRIPTOR;

        goto EXIT;
    }

    int MonitorWidth    = g_MonitorInfo.rcMonitor.right - g_MonitorInfo.rcMonitor.left;
    int MonitorHeight   = g_MonitorInfo.rcMonitor.bottom - g_MonitorInfo.rcMonitor.top;

EXIT:
    return (dwResult);
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
    short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gh_GameWindow, WM_CLOSE, 0, 0);
    }

}

void RenderFrameGraphics(void)
{
    //#6 Where do you draw your backbuffer? To a direct context in a window.
    HDC DeviceContext = GetDC(gh_GameWindow);

    //#6 StretchBlt(...) is from Graphical Device Interface (GDI)(Windows 3.1 era). It stretches your surface over your entire window. Don't use it, use StretchDIBits(...)
    StretchDIBits(DeviceContext,0,0,100,100,0,0,100,100,g_BackBuffer.Memory,&g_BackBuffer.bitMapInfo,DIB_RGB_COLORS,SRCCOPY);

    ReleaseDC(gh_GameWindow, DeviceContext);
}
