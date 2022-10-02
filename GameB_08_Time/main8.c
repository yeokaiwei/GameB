//#8 
#include <stdio.h>

#pragma warning(push,3) 
#include <windows.h>    
#pragma warning(pop)    

#include <stdint.h> 

#include "main8.h"

HWND gh_GameWindow;

BOOL gb_GameIsRunning;

GAMEBITMAP g_BackBuffer;

RECT g_GameWindowSize;

PERFORMANCEDATA gpd_PerformanceData;

//#8 Replaced by gpd_PerformanceData
//MONITORINFO g_MonitorInfo = { sizeof(MONITORINFO) };
//int32_t g_MonitorWidth;
//int32_t g_MonitorHeight;

//MAIN GAMELOOP
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

    //#8 Unions. QPC's QPF is to get the computer's frequency
    //https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
    QueryPerformanceFrequency(&gpd_PerformanceData.PerfFrequency);

    g_BackBuffer.bitMapInfo.bmiHeader.biSize        = sizeof(g_BackBuffer.bitMapInfo.bmiHeader);
    g_BackBuffer.bitMapInfo.bmiHeader.biWidth       = GAME_RESOLUTION_WIDTH;
    g_BackBuffer.bitMapInfo.bmiHeader.biHeight      = GAME_RESOLUTION_HEIGHT;
    g_BackBuffer.bitMapInfo.bmiHeader.biBitCount    = GAME_BPP;
    g_BackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    g_BackBuffer.bitMapInfo.bmiHeader.biPlanes      = 1;
    g_BackBuffer.Memory                             = VirtualAlloc(NULL, GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == g_BackBuffer.Memory)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    //#8 There is no memset_s
    memset(g_BackBuffer.Memory, 0x7F, GAME_DRAWINGAREA_MEMORYSIZE);

    //#8 Message from message
    MSG Message = { 0 };

    gb_GameIsRunning = TRUE;

    //#8 What is our current FPS? This is just 1 frame
    while (TRUE == gb_GameIsRunning)
    {
        QueryPerformanceCounter(&gpd_PerformanceData.FrameStart); //QPC the LARGE_INTEGER FrameStart

        while (PeekMessageA(&Message, gh_GameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();
        
        //#8 Before the Sleep(1).
        QueryPerformanceCounter(&gpd_PerformanceData.FrameEnd); //QPC the LARGE_INTEGER FrameEnd

        gpd_PerformanceData.ElapsedMicroSecondsPerFrame.QuadPart = gpd_PerformanceData.FrameEnd.QuadPart - gpd_PerformanceData.FrameStart.QuadPart;
        
        gpd_PerformanceData.ElapsedMicroSecondsPerFrame.QuadPart *= 1000000;
        
        gpd_PerformanceData.ElapsedMicroSecondsPerFrame.QuadPart /= gpd_PerformanceData.PerfFrequency.QuadPart;

        //#8 Sleep(1) is not what it means. 
        Sleep(1);

        gpd_PerformanceData.TotalFramesRendered++;  

        //#8 CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES is 100. So, we are modulo-ing every 100 frames
        if ( 0 == (gpd_PerformanceData.TotalFramesRendered % CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES) )
        {
            char string[64] = { 0 };

            _snprintf_s(string, _countof(string),_TRUNCATE, "Elapsed microseconds: %lli \n",gpd_PerformanceData.ElapsedMicroSecondsPerFrame.QuadPart);

            OutputDebugStringA(string);
        }


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


    //#8 Previously, if (RegisterClassExA(&windowClass) == 0), just good habits
    if (0 == RegisterClassExA(&windowClass) )
    {
        dw_Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    //#8 Remember to initialize the GAMEPERFORMANCEDATA, else it won't work.
    gpd_PerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    //#8 Remove WS_EX_CLIENTEDGE, set with 0 to remove the white border
    gh_GameWindow = CreateWindowExA(    0,
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

    if (0 == (GetMonitorInfoA(MonitorFromWindow(gh_GameWindow, MONITOR_DEFAULTTOPRIMARY), &gpd_PerformanceData.MonitorInfo)) )
    {
        dw_Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto EXIT;
    }

    gpd_PerformanceData.MonitorWidth = gpd_PerformanceData.MonitorInfo.rcMonitor.right -                                             gpd_PerformanceData.MonitorInfo.rcMonitor.left;
    gpd_PerformanceData.MonitorHeight = gpd_PerformanceData.MonitorInfo.rcMonitor.bottom -                    gpd_PerformanceData.MonitorInfo.rcMonitor.top;

    if (0 == SetWindowLongPtrA(gh_GameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW))
    {
        dw_Result = GetLastError();

        goto EXIT;
    }

    //TRICK
    //if (SetWindowPos(gh_GameWindow, HWND_TOPMOST, g_MonitorInfo.rcMonitor.left, g_MonitorInfo.rcMonitor.top, g_MonitorWidth, g_MonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED))
    if ( NULL == SetWindowPos(   gh_GameWindow, 
                                HWND_TOPMOST, 
                                gpd_PerformanceData.MonitorInfo.rcMonitor.left, 
                                gpd_PerformanceData.MonitorInfo.rcMonitor.top, 
                                gpd_PerformanceData.MonitorWidth, 
                                gpd_PerformanceData.MonitorHeight, 
                                SWP_NOOWNERZORDER | SWP_FRAMECHANGED))
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
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gh_GameWindow, WM_CLOSE, 0, 0);
    }

}

void RenderFrameGraphics(void)
{
    PIXEL32 Pixel = { 0 };

    Pixel.Blue = 0xff;
    Pixel.Green = 0;
    Pixel.Red = 0;
    Pixel.Alpha = 0xff;

    //#8 GAME_RESOLUTION_HEIGHT * GAME_RESOLUTION_WIDTH = 240 *384 = 92,160
    //( GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) ) = 92,160 * 32/8 = 368,640. Crash
    for (int x = 0; x < 50000; x++)
    {
        //#8 memcpy((PIXEL32*)g_BackBuffer.Memory + x, &Pixel, 4);
        // //memcpy((PIXEL32*)g_BackBuffer.Memory + x, &Pixel, sizeof(PIXEL32));
        //(PIXEL32*) is required else error incomplete object type. The compiler has to know how many bytes to offset by in each iteration. How does it know that? Because of the cast, (PIXEL32*)
        //memcpy is not secure, it does not specify the destination buffer size. Use memcpy_s to set the destination size.
        memcpy_s((PIXEL32*)g_BackBuffer.Memory + x, sizeof(PIXEL32), &Pixel, sizeof(PIXEL32));
        
        
        //memcpy((PIXEL32*)g_BackBuffer.Memory + x,  &Pixel, sizeof(PIXEL32)); //#8 Won't throw warnings
    }

    HDC DeviceContext = GetDC(gh_GameWindow);

    StretchDIBits(  DeviceContext,
                    0, 
                    0,
                    gpd_PerformanceData.MonitorWidth, 
                    gpd_PerformanceData.MonitorHeight,
                    0, 
                    0,
                    GAME_RESOLUTION_WIDTH, GAME_RESOLUTION_HEIGHT,
                    g_BackBuffer.Memory, &g_BackBuffer.bitMapInfo,
                    DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(gh_GameWindow, DeviceContext);
}