#pragma once

#define GAME_NAME "Game_B"

//SAL Source Annotated Language. How do you intend to use the parameter? _In_ input variable
LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);

//
DWORD CreateMainGameWindow(void);

//Singleton
BOOL GameIsAlreadyRunning(void); //BOOL is actually an int. BOOL is just for intent.