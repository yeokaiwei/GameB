#pragma once

#define GAME_NAME				"Game_B"
#define GAME_RESOLUTION_WIDTH	384 //True 16:9 that is divisible by 8 
#define GAME_RESOLUTION_HEIGHT	216	//82944 pixels
#define GAME_BPP				32	
#define GAME_DRAWINGAREA_MEMORYSIZE (GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) )

//NES had 40 kilobytes of memory on a 8Mhz processor. Counter-intutively, a 24-bit per pixel is slower than 32-bit because it's not base 2.//#5. Multiple pieces of data inside of 1 thing for the surface, so data structure. typedef doesn't create a new data type. It creates an alias.
typedef struct GAMEBITMAP
{
	BITMAPINFO bitMapInfo; //int height; int width; int bpp (bits per pixel);
	void* Memory;  //Pointer. Size is x86 32bits, x64 64 bits.
}GAMEBITMAP;

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void); 
void ProcessPlayerInput(void);
void RenderFrameGraphics(void); //Assumed to not fail. So no return, void.
