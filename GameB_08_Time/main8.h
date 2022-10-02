#pragma once

#define GAME_NAME								"Game_B"
#define GAME_RESOLUTION_WIDTH					384		
#define GAME_RESOLUTION_HEIGHT					240		
#define GAME_BPP								32	
#define GAME_DRAWINGAREA_MEMORYSIZE				( GAME_RESOLUTION_WIDTH * GAME_RESOLUTION_HEIGHT * (GAME_BPP/8) )
#define CALCULATE_AVERAGE_FPS_EVERY_X_FRAMES	100	

#pragma warning(disable:4820) //Padding warning. Disable warning about structure padding
#pragma warning(disable:5045) //Spectre warning. Disable Spectre/Meltdown PC vulnerability warnings.

//#pragma pack(1). //#8 Ignores padding. Here, we pad as we want the alignment. PC's tend to have large amounts of RAM.
typedef struct GAMEBITMAP //#8 56 bytes
{
	BITMAPINFO bitMapInfo; //44 bytes
						   
	//#8. 4 bytes of Padding was added here. Why? 52/8 is not a multiple of 8. +4 becomes a multiple of 8. Possibly due to IBM standardizing the byte as 8 bits. Padding is bad for embedded systems as it is slow. So, never pad for embedded systems where memory is scarce e.g. 48 kb of RAM. 
	
	void* Memory; //8 bytes
} GAMEBITMAP;

typedef struct PIXEL32
{
	uint8_t Blue;		
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} PIXEL32;

//#8 Group all various global variables into data structure.
typedef struct GAMEPERFORMANCEDATA
{
	//#8 unsigned int 8-bytes. Why is it so big? If you used uint8_t, you would only have 255 frames before it gets a rollback to the first frame. This can run for years. Don't make the data types too big nor too small.
	uint64_t TotalFramesRendered; 

	uint32_t RawFramesPerSecondAverage; //Raw FPS, thousands FPS

	uint32_t CookedFramesPerSecondAverage; //Artificial slow down to 60 FPS

	//#8 LARGE_INTEGER (__int64) is a union of a 64-bit signed integer (QuadPart). A pair of 32-bit integers (LowPart) (HighPart). If you want to perform 64-bit arithmetic on one you need to select the 64-bit int from inside the union. 
	LARGE_INTEGER PerfFrequency; 

	LARGE_INTEGER FrameStart;

	LARGE_INTEGER FrameEnd;

	LARGE_INTEGER ElapsedMicroSecondsPerFrame;

	uint16_t CalculateAverageFPSEveryXFrame; //Mouthful

	//#8 MONITORINFO MonitorInfo = { sizeof(MONITORINFO) }; won't initialize within another data structure.
	MONITORINFO MonitorInfo;
	
	int32_t MonitorWidth;
	
	
	int32_t MonitorHeight;

}PERFORMANCEDATA;

LRESULT CALLBACK MainWindowProc(_In_ HWND hWindow, _In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);


//REFERENCE
//LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
//LARGE_INTEGER Frequency;
//
//QueryPerformanceFrequency(&Frequency);
//QueryPerformanceCounter(&StartingTime);
//
//// Activity to be timed
//
//QueryPerformanceCounter(&EndingTime);
//ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
//
//
////
//// We now have the elapsed number of ticks, along with the
//// number of ticks-per-second. We use these values
//// to convert to the number of elapsed microseconds.
//// To guard against loss-of-precision, we convert
//// to microseconds *before* dividing by ticks-per-second.
////
//
//ElapsedMicroseconds.QuadPart *= 1000000;
//ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;