//#5
#include <stdio.h>

#pragma warning(push,0) 
#include <windows.h>    
#pragma warning(pop)    

#include "main5.h"

//HANDLE gh_GameWindow;
HWND gh_GameWindow;

BOOL gb_GameIsRunning;

GAMEBITMAP gbm_DrawingSurface; //drawingSurface.height = 480; drawingSurface.width = 640;


//#5 The WINAPI __stdcall is required for x86. WINAPI calls are __stdcall. Calling conventions are the protocol that 1 function uses to call another function and that called function uses to then return to the previous function. In x86, Run to Cursor HERE, then Debug->Window->Disassembly.
// Caller and Callee. In __stdcall, stack cleanup is performed by the called function. In __cdecl, stack cleanup is performed by the caller.
// CALLBACK and WINAPI will translate to __stdcall. In x64, they use __fastcall instead. __fastcall tries to stick data into the CPU registers instead of the stack frame.
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
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

    //#5. 1st, fill out the size of the data structure. Old NES resolution of 256*240. SNES 256*224, smaller but more colours. Modern aspect ratio 16:9. Makes this compatible with WinAPI
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biSize          = sizeof(gbm_DrawingSurface.bitMapInfo.bmiHeader);
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biWidth         = GAME_RESOLUTION_WIDTH; 
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biHeight        = GAME_RESOLUTION_HEIGHT; 
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biBitCount      = GAME_BPP;
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biCompression   = BI_RGB; //NO COMPRESSION. 32 bits per pixel only.
    gbm_DrawingSurface.bitMapInfo.bmiHeader.biPlanes        = 1; //Number of planes for the target device. This value must be set to 1.
    gbm_DrawingSurface.Memory = VirtualAlloc(NULL,GAME_DRAWINGAREA_MEMORYSIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    if(NULL == gbm_DrawingSurface.Memory)
    {
        //Allocate memory. VirtualAlloc is for LARGE allocation, big one-time allocation. NULL lets Windows Manager allocate the memory. sizeof the memory, MEM_RESERVE|MEM_COMMIT type of allocation, PAGE_READWRITE protection of the memory is read/write
        //Could fail if memory is insufficient, then no memory address returned. 
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        //return (0); or
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

        //#5 Avoid parameter passing to ensure speedy game loop
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

    gh_GameWindow = CreateWindowExA(    WS_EX_CLIENTEDGE,
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
    short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gh_GameWindow, WM_CLOSE, 0, 0);
    }

}

void RenderFrameGraphics(void)
{
    //#5. Need a drawing surface (canvas) stored in memory. Every frame we copy-paste this entire surface all at once onto the screen. If we use per-pixel blitting, flickering will occur. 

}


//#5 Variadic. Variadic means it takes in an infinite amount of parameters, __cdecl is variadic. printf is __cdecl. 
//e.g. printf("%s %s %s\n", a , b, c, d); //Keeps on going 

//#5 Buffer Overflow
////unsigned int overflow = 0xffffffff; //Won't trigger overflow. Has Warning!
//unsigned int overflow = 0;
////overflow = 4294967295 + 1; //Overflow
//overflow = 4294967296; //Overflow
//
//unsigned char overchar = 0;
//overchar = 255 + 1;//Shows 0
//overchar = 256;//Shows 0
//
//char a = 255; //-1
//a = 255 + 1; //-1
//
//char b = 0;
//b = -128 -2;//126

//#5
//      x86 Disassembly
//      00C11C50  push        ebp
//      00C11C51  mov         ebp, esp
//      00C11C53  sub         esp, 0E8h
//      00C11C59  push        ebx
//      00C11C5A  push        esi
//      00C11C5B  push        edi
//      00C11C5C  lea         edi, [ebp - 28h]
//      00C11C5F  mov         ecx, 0Ah
//      00C11C64  mov         eax, 0CCCCCCCCh
//      00C11C69  rep stos    dword ptr es : [edi]
//      00C11C6B  mov         eax, dword ptr[__security_cookie(0C1A004h)]
//      00C11C70 xor eax, ebp
//      00C11C72  mov         dword ptr[ebp - 4], eax
//      00C11C75  mov         ecx, offset _64B72FB4_main5@c(0C1C00Fh)
//      00C11C7A  call        @__CheckForDebuggerJustMyCode@4 (0C11334h) //Debugger skips over code that is not your code
//      x86, Properties-> C/C++ -> Support Just My Code Debugging -> No
//      009D1C10  push        ebp
//      009D1C11  mov         ebp, esp
//      009D1C13  sub         esp, 0E8h
//      009D1C19  push        ebx
//      009D1C1A  push        esi
//      009D1C1B  push        edi
//      009D1C1C  lea         edi, [ebp - 28h]
//      009D1C1F  mov         ecx, 0Ah
//      009D1C24  mov         eax, 0CCCCCCCCh
//      009D1C29  rep stos    dword ptr es : [edi]
//      009D1C2B  mov         eax, dword ptr[__security_cookie(09DA004h)]
//      009D1C30  xor eax, ebp
//      009D1C32  mov         dword ptr[ebp - 4], eax

//x64, read right-to-left
    //      00007FF7CD221BA0  mov         dword ptr[rsp + 20h], r9d //r9
    //      00007FF7CD221BA5  mov         qword ptr[rsp + 18h], r8 //r8
    //      00007FF7CD221BAA  mov         qword ptr[rsp + 10h], rdx //Register D
    //      00007FF7CD221BAF  mov         qword ptr[rsp + 8], rcx //Programmers call it register C
    //      00007FF7CD221BB4  push        rbp
    //      00007FF7CD221BB5  push        rdi
    //      00007FF7CD221BB6  sub         rsp, 148h
    //      00007FF7CD221BBD  lea         rbp, [rsp + 30h]
    //      00007FF7CD221BC2  lea         rdi, [rsp + 30h]
    //      00007FF7CD221BC7  mov         ecx, 16h
    //      00007FF7CD221BCC  mov         eax, 0CCCCCCCCh
    //      00007FF7CD221BD1  rep stos    dword ptr[rdi]
    //      00007FF7CD221BD3  mov         rcx, qword ptr[rsp + 168h]
    //      00007FF7CD221BDB  mov         rax, qword ptr[__security_cookie(07FF7CD22D008h)]
    //      00007FF7CD221BE2 xor rax, rbp
    //      00007FF7CD221BE5  mov         qword ptr[rbp + 108h], rax