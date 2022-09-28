//#2 
// Action: Change to main.c from main.cpp
// Note:	Debug, Properties-> Advanced -> Whole Program Optimization -> No Whole Program Optimization 
//			Release,Properties-> Advanced -> Whole Program Optimization -> Use Link Time Code Generation (All optimization)
// 
// Action:	Properties-> Advanced -> Character Set -> Use MultiByte Character Set (ASCII)
// 
// Action:	Properties-> C/C++ -> General-> Warning Level-> Enable All Warnings /Wall
// 
// Note:    #5. Don't add in the Debugging in assembly. Removes @__CheckForDebuggerJustMyCode@4 (0C11334h)
// Action:  Properties-> C/C++ -> Support Just My Code Debugging -> No
// 
// Note: Debug makes it easier to share/portable, but it's slow. 
// 		Debug, C/C++ -> Code Generation -> Runtime Library -> Multi-threaded Debug /MTd (Static link, big file) e.g. 1.0 MB. DLLs              embedded into the executable
// 		Release, C/C++ -> Code Generation -> Runtime Library -> Multi-threaded DLL /MD (Dynamic? Small with DLLs in Windows e.g 57kb +          DLL's in Windows). If someone else didn't have the same DLLs, it would fail.
//
// Action: Release, C/C++ -> Code Generation -> Runtime Library -> Multi-threaded (/MT)
//
// Note: All the above options translate into command line code. IDE is just a graphical overlay. C/C++ -> Command Line. Change configuration to Release and x64.
// See this "/permissive- /ifcOutput "x64\Release\" /GS /GL /Wall /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Fd"x64\Release\vc142.pdb" /Zc:inline /fp:precise /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MT /FC /Fa"x64\Release\" /EHsc /nologo /Fo"x64\Release\" /Fp"x64\Release\GameB_02_Win32Basics.pch" /diagnostics:column"
//
// Note: Linker-> System-> SubSystem-> Console (/SUBSYSTEM:CONSOLE). Currently, a console app. Will throw LNK2019 and LNK1102 errors. 
// Action: Change to Windows (/SUBSYSTEM:WINDOWS) to resolve
//
// Note: Conversion to Windows app, WinMain. There is a wWinMain (the extra w), wide Unicode. Don't use it
//  HINSTANCE hInstance is a handle or memory address to the program
//  HINSTANCE hPrevInstance is the previous instance.
//  PSTR lpCmdLine, is a char* pointer to a string to a commandline or the entire commandline arguments
//  INT nCmdShow Controls how the window is to be shown. Ignored the first time. Maximize or minimize window.
//  Every Windows program starts with WinMain e.g. Notepad, Calculator
//  INT is still int

#include <stdio.h>

//Precompiler directives.  #define WIN32_LEAN_AND_MEAN is a workaround
#pragma warning(push,3)  //Adjusts warning levels. Save the current warning level, set current level to 0. Disable all warning. Instead, drop to 3. Else, no memcpy warning.

#include <windows.h>    //Then, add windows.h

#pragma warning(pop)    //Then, pop out the warning level and reset to /Wall

//LRESULT is a long pointer. CALLBACK is __stdcall, a calling convention. Another cc is __cdecl
LRESULT CALLBACK MainWindowProc(HWND hWindow,        // handle to window
                                UINT uMessage,        // message identifier
                                WPARAM wParam,    // first message parameter
                                LPARAM lParam);   // second message parameter

DWORD CreateMainGameWindow();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    //HANDLE z = hInstance; // Warning	C4189	'z': local variable is initialized but not referenced	GameB_02_Win32Basics	 AND  Warning	C4100	'nCmdShow': unreferenced formal parameter	GameB_02_Win32Basics	


    //Actiom: To solve C4100. Use UNREFERENCED_PARAMETER to tell the compiler that you are not using them. 
    //In C++, you can just remove the reference name e.g. HINSTANCE, HINSTANCE, PSTR, INT. 
    //In C, you can't do that, must use UNREFERENCED_PARAMETER.
    //UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    //Note: Registering a new windowclass. If you don't reference a variable, Release will optimize it out. You can't reach it. Debug fill in random stuff like 0xcccccccccccccccc. The values will be red because it just got filled in. If no zero-initialization ={0}, the debugger will random fill and skip to the next step. {0} is for data structs. Simple things like int can be int z=0;
    
    HWND hWindow;

    WNDCLASSEXA windowClass = { 0 }; //No typedef from WNDCLASSEX. Direct. 

        //WNDCLASSEXA windowClass = { sizeof(WNDCLASSEX) }; You can set the size like this.
        windowClass.cbSize = sizeof(WNDCLASSEX); //1st, fill in the size of the thing. cbytes means count in bytes.
    
        windowClass.style = 0;
    
        windowClass.lpfnWndProc = MainWindowProc; //Windows of windowClass, they will follow this procedure MainWindowProc
    
        windowClass.cbClsExtra = 0;
    
        windowClass.cbWndExtra = 0;
    
        //windowClass.hInstance = hInstance; //Same as = GetModuleHandleA(NULL); Your Proc might be in another DLL, so you might need to find the module (dll or exe) handle to load it.
        windowClass.hInstance = GetModuleHandleA(NULL);
    
        windowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION); //Direct. Null is 0.
    
        windowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

        windowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    
        windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
        windowClass.lpszMenuName = NULL;
    
        windowClass.lpszClassName = "GAME_B_WINDOWCLASS";
    
        

    //if (!RegisterClassEx(&windowClass)). Not clear.
    if (RegisterClassExA(&windowClass)==0)
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return (0);
    }

    hWindow = CreateWindowExA(   WS_EX_CLIENTEDGE,      //Window style
                                "GAME_B_WINDOWCLASS",   //Defined window class
                                "The title of my window", //Title
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,  //WS_OVERLAP is 0. No min, max, close. WS_OVERLAPPEDWINDOW is a combo
                                CW_USEDEFAULT, CW_USEDEFAULT, //Position
                                240, 120,   //Size
                                NULL, NULL, hInstance, NULL);   //

    //Unnecessary with WS_VISIBLE. Currently, X won't clsoe the program.
    //ShowWindow(hWindow, TRUE);


    MSG message = { 0 };

    while (GetMessageA(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageA(&message); //Dispatch message to which window class and which proc to use
    }

    return (0);
}

LRESULT CALLBACK MainWindowProc( HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)    
{
    LRESULT lResult = { 0 };

    switch (uMessage)
    {
    default:
    {
        lResult = DefWindowProcA(hWindow, uMessage, wParam, lParam);//What if it's not 0?
    }
    }
    return lResult;
}

//while (TRUE) //true won't work, TRUE will
//{

//}

//switch (uMessage)
//{
//case WM_CREATE:
//    // Initialize the window. 
//    return 0;
//
//case WM_PAINT:
//    // Paint the window's client area. 
//    return 0;
//
//case WM_SIZE:
//    // Set the size and position of the window. 
//    return 0;
//
//case WM_DESTROY:
//    // Clean up window-specific data objects. 
//    return 0;
//
//    // 
//    // Process other messages. 
//    // 
//
//default:
//    return DefWindowProc(hWindow, uMessage, wParam, lParam);
//}

DWORD CreateMainGameWindow()
{
    DWORD dResult = ERROR_SUCCESS;

    return (dResult); 
}