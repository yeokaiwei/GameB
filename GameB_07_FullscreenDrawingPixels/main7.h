#pragma once

#define GAME_NAME						"Game_B"
#define GAME_RESOLUTION_WIDTH			384		
#define GAME_RESOLUTION_HEIGHT			216		//#6 16*16 tiles. 216/16 is 13.5.
#define GAME_BPP						32	
#define GAME_DRAWINGAREA_MEMORYSIZE		( GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) )

typedef struct GAMEBITMAP
{
	BITMAPINFO bitMapInfo;
	void* Memory;
} GAMEBITMAP;

//#6 32 bit pixel. There are many types of pixels. BGRA is a 1972
typedef struct PIXEL32
{
	uint8_t Blue;		//uint8_t means unsigned char. unsinged 8 bit integer. Requires <stdint>
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} PIXEL32;

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);
