#pragma once

#define GAME_NAME								"Game_B"
#define GAME_RESOLUTION_WIDTH					384		
#define GAME_RESOLUTION_HEIGHT					240		
#define GAME_BPP								32	
#define GAME_DRAWINGAREA_MEMORYSIZE				( GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) )
#define CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES	100	
#define TARGET_MICROSECONDS_PER_FRAME			16667 //#9 16667 microseconds is 16.667 milliseconds.

#pragma warning(disable:4820) 
#pragma warning(disable:5045) 
#pragma warning(disable:4710)	//#9 Disable warning about function not inlining 


typedef struct GAMEBITMAP
{
	BITMAPINFO bitMapInfo;

	void* Memory;
} GAMEBITMAP;

typedef struct PIXEL32
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} PIXEL32;

typedef struct GAMEPERFORMANCEDATA
{
	uint64_t TotalFramesRendered;

	//uint32_t RawFramesPerSecondAverage;
	float RawFramesPerSecondAverage;

	//uint32_t CookedFramesPerSecondAverage; 
	float CookedFramesPerSecondAverage;

	//#9 Changed LARGE_INTEGER to int64_t
	int64_t PerfFrequency;

	//LARGE_INTEGER FrameStart; //#9 Removed. Global variables can be accidentally updated.

	//LARGE_INTEGER FrameEnd;

	//#9 Changed LARGE_INTEGER to int64_t
	//int64_t ElapsedMicroSecondsPerFrame;

	uint16_t CalculateAverageFPSEveryXFrame;

	MONITORINFO MonitorInfo;

	int32_t MonitorWidth;

	int32_t MonitorHeight;

}PERFORMANCEDATA;

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);


