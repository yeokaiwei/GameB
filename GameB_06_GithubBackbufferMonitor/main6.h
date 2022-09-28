#pragma once

#define GAME_NAME						"Game_B"
#define GAME_RESOLUTION_WIDTH			384 
#define GAME_RESOLUTION_HEIGHT			216	
#define GAME_BPP						32	
#define GAME_DRAWINGAREA_MEMORYSIZE		( GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) )

typedef struct GAMEBITMAP
{
	BITMAPINFO bitMapInfo; 
	void* Memory;  
} GAMEBITMAP;

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);