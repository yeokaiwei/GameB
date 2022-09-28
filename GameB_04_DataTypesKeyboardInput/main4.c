#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include "main4.h"

HANDLE gh_GameWindow;

//#4 Old school C is 0 or !0. Global variables are guaranteed to be initialized to 0 i.e. {0}
BOOL gb_GameIsRunning;


//#4 Assumed to never fail, so return void.
void ProcessPlayerInput();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
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

    //#4 Creating the main game window here creates no handle for PeekMessageA()
    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto EXIT;
    } 

    MSG message = { 0 };

    gb_GameIsRunning = TRUE;

    //#4 Good habit. TRUE== gb_GameIsRunning. Won't have TRUE= gb_GameIsRunning. 1 frame.
    while (TRUE== gb_GameIsRunning )
    {
        //0,0 is for filtering for a specific type, No PM_RETRIEVE. PM_REMOVE. It will look and remove the message. PM_REMOVE Messages are removed from the queue after processing by PeekMessage. PM_NOREMOVE Messages are not removed from the queue after processing by PeekMessage. 
        while (PeekMessageA(&message, gh_GameWindow , 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&message);
        }

        //60 fps so 16.67 milliseconds
        ProcessPlayerInput();

        //RenderFrameGraphics();

        //#4. PROBLEM 1. High CPU core usage. Sleep(0) is meant to yield the CPY core to another thread. However, it only yields it for a fraction of time. Sleep(1) makes a vast difference as it reduces CPU time by a lot. If you increase Sleep (1000), CPU will be very low but then you can't hit 60fps. Sleep can't be too high or too low.
        //#4. PROBLEM 2. Not fast enugh returns. The other thread might not return the thread fast enough. No predictability. No smooth graphics, jittery.
        Sleep(1);
    }

    //#4 GetMessageA loop blocks. It halts execution waiting from windows message to come in. Only useful for simple apps. BAD! We should wait for a message to come in. When you hover over a window and it lights up, the mouse is sending messages to the window. 
    //You want the game to keep moving even if no keys are pressed e.g. Move a character amd stop, and then the game pauses. Need a constant steady main game loop.
    //while (GetMessageA(&message, NULL, 0, 0) > 0)
    //{
    //    TranslateMessage(&message); //Translates virtual-key messages into character messages.
    //    DispatchMessageA(&message);
    //}

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
        gb_GameIsRunning = FALSE;
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

    //HWND hWindow;

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

    gh_GameWindow = CreateWindowExA(  WS_EX_CLIENTEDGE,
                                GAME_NAME"_WINDOWCLASS",
                                "Window Title",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                240, 120,
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

void ProcessPlayerInput()
{
    //GetAsyncKeyState() return TRUE (key is pressed down) or FALSE (key is up). SHORT is half an INT.
    short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        //Send our game window a message when the ESC key is pressed. wp and lp are 0 since no additional params
        SendMessageA(gh_GameWindow,WM_CLOSE,0,0); 
    }

}

//#4 Data Types
//An integer is 4bytes = 4*8 = 32 bits. 4.5 billion
//                      42 1  
//                      00 0521  
//                      94 2152 631                
//                      68 4268 4268-5420 
// --------------------------------------
//0000-0000 0000-0000 0000-0000 0000-0000