//#6 Github. Gitignore tells Git which files to ignore. If you choose Visual Studio, it will upload .c, .cpp, .h files but ignore the things like bitmaps, encryption keys to web servers and backend databases, sound files, assets.

#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include "main6.h"

HWND gh_GameWindow;

BOOL gb_GameIsRunning;

//#6 gbm_DrawingSurface is the backbuffer
GAMEBITMAP gbm_DrawingSurface;


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
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

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto EXIT;
    }

    gbm_DrawingSurface.bitMapInfo.bmiHeader.biSize = sizeof(gbm_DrawingSurface.bitMapInfo.bmiHeader);
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biWidth = GAME_RESOLUTION_WIDTH;
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biHeight = GAME_RESOLUTION_HEIGHT;
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biCompression = BI_RGB; 
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biPlanes = 1; 
    gbm_DrawingSurface.Memory = VirtualAlloc(NULL, GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == gbm_DrawingSurface.Memory)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto EXIT;
    }


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
    DWORD dResult = ERROR_SUCCESS;

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

    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    windowClass.lpszMenuName = NULL;

    windowClass.lpszClassName = GAME_NAME"_WINDOWCLASS";

    if (RegisterClassExA(&windowClass) == 0)
    {
        dResult = GetLastError();

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

    if (gh_GameWindow == NULL)
    {
        dResult = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

EXIT:
    return (dResult);
}

BOOL GameIsAlreadyRunning(void)
{
    HANDLE mutex = NULL;

    mutex = CreateMutexA(NULL, FALSE, GAME_NAME"_GameMutex");

   
    if (GetLastError() == ERROR_ALREADY_EXISTS)
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


}
