//#9 Measuring 60 fps. Timekeeping. Why Win32 over Universal Windows Platform (UWP)? UWP is sandboxed. It is abstracted, can't interfere with the OS, can't track data and everthing is virtualized. UWP is mainly for Windows Store apps and Xbox compatibility. UWP has been superseded. Win32 is for the highest level of performance.
//#9 This tutorial ends broken.

#include <stdio.h>

#pragma warning(push,3) 
#include <windows.h>    
#pragma warning(pop)    

#include <stdint.h> 

#include "main9.h"

HWND gh_GameWindow;

BOOL gb_GameIsRunning;

GAMEBITMAP g_BackBuffer;

RECT g_GameWindowSize;

PERFORMANCEDATA gpd_PerformanceData;

//MAIN GAMELOOP
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow)
{
    int64_t FrameStart = 0;
    int64_t FrameEnd = 0;
    int64_t ElapsedMicroSecondsPerFrame = 0;
    int64_t ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;//#9
    int64_t ElapsedMicroSecondsPerFrameAccumulatorCooked = 0; //#9

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    MSG Message = { 0 };

    if (TRUE == GameIsAlreadyRunning())
    {
        MessageBoxA(NULL, "Another instance of the program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    if (ERROR_SUCCESS != CreateMainGameWindow())
    {
        goto EXIT;
    }

    //#9 Setting the global system timer resource to 1 ms instead of 15 ms. BAD! Counter-intuitively, it makes your system slower as you context switch more. So, don't use it, just for knowledge.
    //timeBeginPeriod(1); 

    //#9 warning C4133: 'function': incompatible types - from 'int64_t *' to 'LARGE_INTEGER *'. Remember, LARGE_INTEGER*
    QueryPerformanceFrequency((LARGE_INTEGER*)&gpd_PerformanceData.PerfFrequency);

    g_BackBuffer.bitMapInfo.bmiHeader.biSize = sizeof(g_BackBuffer.bitMapInfo.bmiHeader);
    
    g_BackBuffer.bitMapInfo.bmiHeader.biWidth = GAME_RESOLUTION_WIDTH;
    
    g_BackBuffer.bitMapInfo.bmiHeader.biHeight = GAME_RESOLUTION_HEIGHT;
    
    g_BackBuffer.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    
    g_BackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    
    g_BackBuffer.bitMapInfo.bmiHeader.biPlanes = 1;
    
    //#9 VirtualAlloc is not freed. Why? If you are in User Mode and you use the memory for the duration of the app, it doesn't matter if you free it. Windows will automatically clean it up. However, if you are using it for a short-term and you keep switching things in and out, e.g. loading multiple sprites, you will need to free the memory manually.
    g_BackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == g_BackBuffer.Memory)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    memset(g_BackBuffer.Memory, 0x7F, GAME_DRAWINGAREA_MEMORYSIZE);

    gb_GameIsRunning = TRUE;

    while (TRUE == gb_GameIsRunning)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart); 

        while (PeekMessageA(&Message, gh_GameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
        //-------------------------------------------------------------------------------------------------//

        //39 LARGE_INTEGER was replacecd by int64_t, no need for QuadPart as it's not an union
        ElapsedMicroSecondsPerFrame = FrameEnd - FrameStart;

        ElapsedMicroSecondsPerFrame *= 1000000;

        ElapsedMicroSecondsPerFrame /= gpd_PerformanceData.PerfFrequency;

        //#9 Moved before Sleep(1)
        gpd_PerformanceData.TotalFramesRendered++;

        //#9 Add the elapsed time to the accumulator. Moved before Sleep(1)
        ElapsedMicroSecondsPerFrameAccumulatorRaw += ElapsedMicroSecondsPerFrame;

        //#9 While (FrameEnd - FramStart) is less than 16667 microseconds i.e. you still have some remaining time budget
        while (ElapsedMicroSecondsPerFrame <= TARGET_MICROSECONDS_PER_FRAME)
        {
            Sleep(0);
            
            ElapsedMicroSecondsPerFrame = FrameEnd - FrameStart;

            ElapsedMicroSecondsPerFrame *= 1000000;

            ElapsedMicroSecondsPerFrame /= gpd_PerformanceData.PerfFrequency; 

            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
        }
        //#9 Inline means you want to run a function as fast as possible. Inline copy-pastes that code directly into your code instead of calling that function. Why? It costs CPU time. Due to memory access and you have to push things to the stack and reconfigure the stack before you can call another function. Inlining just copy-pastes the code inline your code.
        //You can force the compiler to inline with __forceinline and __alwaysinline. The downside is that your .exe will be larger. E.g. If functionA has functionB inside of it, instead of functionB(); you will have the entire code in its place.

        //#9 Taking measurements 
        ElapsedMicroSecondsPerFrameAccumulatorCooked += ElapsedMicroSecondsPerFrame;
        
        
        //#9 Every 100 frames, you modulo to calculate the average. CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES is 100
        if (0 == (gpd_PerformanceData.TotalFramesRendered % CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES))
        {
            int64_t AverageMicroSecondsPerFrameRaw = ElapsedMicroSecondsPerFrameAccumulatorRaw / CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES;
            int64_t AverageMicroSecondsPerFrameCooked = ElapsedMicroSecondsPerFrameAccumulatorCooked / CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES;

            //#9
            gpd_PerformanceData.RawFramesPerSecondAverage = 1.0f / (ElapsedMicroSecondsPerFrameAccumulatorRaw / 60 * 0.000001f);
            gpd_PerformanceData.CookedFramesPerSecondAverage = 1.0f / (ElapsedMicroSecondsPerFrameAccumulatorCooked / 60 * 0.000001f);

            //#9 *0.001f is /1000. No need to declare a variable. Just leave it as an equation.
            //float AverageMilliSecondsPerFrame = AverageMicroSecondsPerFrame *0.001f;
            //#9 Buffer is bigger now. 64 to 128
            char string[128] = { 0 };

            //#9 Use sprintf to format data and Error code handling
            //_snprintf_s(string, _countof(string), _TRUNCATE, "Elapsed microseconds: %lli \n", ElapsedMicroSecondsPerFrame);
            //_snprintf_s(string, _countof(string), _TRUNCATE, "Avg milliseconds/frame: %0.02f \t Average FPS: %0.01f\n", (0.001f*AverageMicroSecondsPerFrame ), ( 1.0f/(ElapsedMicroSecondsPerFrameAccumulator/60 * 0.000001f) ));
            //_snprintf_s(string, 
            //            _countof(string), 
            //            _TRUNCATE, 
            //            "Avg milliseconds/frame: %0.02f \t Average cooked FPS: %0.01f \t Average raw FPS: %0.01f\n", 
            //            (0.001f*AverageMicroSecondsPerFrameRaw ), 
            //            gpd_PerformanceData.CookedFramesPerSecondAverage, 
            //            gpd_PerformanceData.RawFramesPerSecondAverage );
            _snprintf_s(string,
                _countof(string),
                _TRUNCATE,
                "Avg milliseconds/frame: %0.02f \t Average cooked FPS: %0.01f \t Average raw FPS: %0.01f\n",
                AverageMicroSecondsPerFrameRaw,
                gpd_PerformanceData.CookedFramesPerSecondAverage,
                gpd_PerformanceData.RawFramesPerSecondAverage);


            OutputDebugStringA(string);

            //#9 Remember to reset the accumulators to 0
            ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;
            ElapsedMicroSecondsPerFrameAccumulatorCooked = 0;
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
    //#9 For normal Win32 apps, case WM_PAINT: will be called whenever they need to repaint the window. Efficient but not for games.
    
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

    if (0 == RegisterClassExA(&windowClass))
    {
        dw_Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }

    gpd_PerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    gh_GameWindow = CreateWindowExA(0,
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

    if (0 == (GetMonitorInfoA(MonitorFromWindow(gh_GameWindow, MONITOR_DEFAULTTOPRIMARY), &gpd_PerformanceData.MonitorInfo)))
    {
        dw_Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto EXIT;
    }

    gpd_PerformanceData.MonitorWidth = gpd_PerformanceData.MonitorInfo.rcMonitor.right - gpd_PerformanceData.MonitorInfo.rcMonitor.left;
    gpd_PerformanceData.MonitorHeight = gpd_PerformanceData.MonitorInfo.rcMonitor.bottom - gpd_PerformanceData.MonitorInfo.rcMonitor.top;

    if (0 == SetWindowLongPtrA(gh_GameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW))
    {
        dw_Result = GetLastError();

        goto EXIT;
    }

    //TRICK
    if (0 == SetWindowPos(gh_GameWindow,
        HWND_TOPMOST,
        gpd_PerformanceData.MonitorInfo.rcMonitor.right,
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

    //#9 If you hide the for loop, the millisecondss per frame will drop and the background will be gray. E.g. 3.0 ms to 0.34 ms. In Win32 apps, like Calculator, Calculator only updates parts that have changed. Refer to MainGameWindowProc. In games, you want something similar. 
    for (int x = 0; x < GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT; x++)
    {
        memcpy_s((PIXEL32*)g_BackBuffer.Memory + x, sizeof(PIXEL32), &Pixel, sizeof(PIXEL32));
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
                    g_BackBuffer.Memory, 
                    &g_BackBuffer.bitMapInfo,
                    DIB_RGB_COLORS, 
                    SRCCOPY);

    ReleaseDC(gh_GameWindow, DeviceContext);
}