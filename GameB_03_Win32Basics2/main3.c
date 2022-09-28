//#3
//Refer to #2 for project settings and notes.

#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include "main3.h"//All forward definitions here. MUST BE AFTER #include <windows.h> as there are some functions you need. Only 1 copy of each function in main3.c.  


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
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

    MSG message = { 0 };

    //Keeps running forever unless it GetMessageA(...) returns a 0
    while( GetMessageA(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageA(&message); 
    }

    //while (PeekMessage(&MainGameWindowMessage, gMainGameWindowHandle, 0, 0, PM_RETRIEVE))
    //{
    //    DispatchMessageA(&message);
    //}

EXIT:
    return (0);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT lResult = { 0 };



    switch (uMessage)
    {
        //Any code outside of a case is ignored
        case WM_CLOSE: //If an application processes this message, it should return 0
        {
            PostQuitMessage(0); //Sends 0 to GetMessageA(...) which will end the while loop. Else, program keeps running in bg.
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

    HWND hWindow;

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
        //If windowClass registration fails, GetLastError() and record it into dResult for returning as DWORD.
        dResult = GetLastError();

        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        
        //Since error, skip window creation, jump and just return dResult
        goto EXIT; 
    }

    hWindow = CreateWindowExA(  WS_EX_CLIENTEDGE,
                                GAME_NAME"_WINDOWCLASS",
                                "Window Title",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                240, 120,
                                NULL, NULL, windowClass.hInstance, NULL);

    if(hWindow == NULL)
    {
        dResult = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

EXIT:
    return (dResult);
}

//Prevents multiple windows. Can be circumvented using ProcessExploer and closing the mutex
BOOL GameIsAlreadyRunning(void)
{
    //A mutex is a piece of memory that is used to gate access to resources. Only 1.
    HANDLE mutex = NULL;
    
    //Attempt to create a mutex. NULL, the handle cannot be inherited by child processes. FALSE,  calling thread does not obtain ownership of the mutex.  GAME_NAME"_GameMutex" is the name of the mutex. object
    mutex = CreateMutexA(NULL, FALSE, GAME_NAME"_GameMutex");

    //If the mutex already exists, 
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }

}


//Buffer overrun
//char buff[12] = { 0 };
//_itoa_s(uMessage, buff, _countof(buff), 10); //integer to ascii characters. radix base 10
//OutputDebugStringA(buff);
//OutputDebugStringA('\n');