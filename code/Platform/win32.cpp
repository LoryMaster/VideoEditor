#include "Platform\lsWindows.h"
#include "Platform\win32.h"
#include "MKVProcessing\MKVParser.h"

#define LS_CRT_IMPLEMENTATION
#include "tools\lsCRT.h"

internal ULONGLONG Win32_Time()
{
	FILETIME FileTime = {};
	GetSystemTimeAsFileTime(&FileTime);

	ULARGE_INTEGER Time = {};
	Time.LowPart = FileTime.dwLowDateTime;
	Time.HighPart = FileTime.dwHighDateTime;

	ULONGLONG Result = (Time.QuadPart / 10000000);
	return Result;
}

internal VOID Win32_AllocateMemory(Memory *Memory, s32 Size, s32 MinimumAllocationSize)
{
	Memory->Size = Size;
	Memory->UsedMemory = 0;
	Memory->RemainingMemory = Size;
	Memory->MinimumAllocationSize = MinimumAllocationSize;

	if ((Memory->BeginPointer = VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) == NULL)
	{
		DWORD Error = GetLastError();
		LogErrori("When calling VirtualAlloc in Win32_AllocateMemory got error: ", Error);
	}
}

internal DWORD Win32_GiveMemory(Memory *Memory, VOID **BasePointer, s32 Size)
{
	if ((Memory->RemainingMemory - Size) > 0)
	{
		if (Size < Memory->MinimumAllocationSize)
		{
			*BasePointer = (VOID *)((u8 *)Memory->BeginPointer + Memory->UsedMemory);
			Memory->UsedMemory += Memory->MinimumAllocationSize;
			Memory->RemainingMemory -= Memory->MinimumAllocationSize;
		}
		else
		{
			*BasePointer = (VOID *)((u8 *)Memory->BeginPointer + Memory->UsedMemory);
			Memory->UsedMemory += Size;
			Memory->RemainingMemory -= Size;
		}

		return TRUE;
	}
	else
	{
		//@TODO: Fix Crash Path with dinamically growing Arena(IF STRICTLY NECESSARY!)
		//Assert("Tried To Allocate More Memory than the Arena had to give!!" == 0)
		return FALSE;
	}
}

internal VOID Win32_ReadTextFile(Memory *Memory, char *Path, char **Dest)
{
	DWORD Error = 0;
	HANDLE FileHandle = 0;
	if ((FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		LogErrori("When creating a file handle got error: ", Error);
	}

	LARGE_INTEGER FileSize = {};
	if (GetFileSizeEx(FileHandle, &FileSize) == 0)
	{
		Error = GetLastError();
		LogErrori("When getting file size got error: ", Error);
	}

	if (Win32_GiveMemory(Memory, (void **)(Dest), FileSize.LowPart) == FALSE)
	{
		OutputDebugStringA("Couldn't Allocate The memory to store File contents into.\n");
		return;
	}

	DWORD BytesRead = 0;
	if (ReadFile(FileHandle, *Dest, FileSize.LowPart, &BytesRead, NULL) == FALSE)
	{
		Error = GetLastError();
		LogErrori("When Reading contents of a file got error: ", Error);
	}
	else
	{
		if (BytesRead != FileSize.LowPart)
		{
			Assert("Bytes read when reading entire file don't equal the file size!!" == 0)
		}
	}

	if (CloseHandle(FileHandle) == FALSE)
	{
		OutputDebugStringA("Couldn't close file handle at the end of Win32_ReadEntireFile function.\n");
	}
}

internal VOID Win32_ReadEntireFile(const char *Path, FileInfo *FileInfo, Memory *Memory)
{
	DWORD Error = 0;
	HANDLE FileHandle = 0;
	if ((FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		LogErrori("When creating a file handle got error: ", Error);
	}

	LARGE_INTEGER FileSize = {};
	if (GetFileSizeEx(FileHandle, &FileSize) == 0)
	{
		Error = GetLastError();
		LogErrori("When getting file size got error: ", Error);
	}

	s32 PathLen = ls_len((char *)Path);
	char ext[4];
	ext[0] = Path[PathLen - 3];
	ext[1] = Path[PathLen - 2];
	ext[2] = Path[PathLen - 1];
	ext[3] = 0;

	if (ls_strcmp(ext, "bmp") == 0)
	{
		FileInfo->FileType = BITMAP;
	}
	else if (ls_strcmp(ext, "mkv") == 0)
	{
		FileInfo->FileType = MKV;
	}

	if (Win32_GiveMemory(Memory, &FileInfo->data, FileSize.LowPart) == FALSE)
	{
		OutputDebugStringA("Couldn't Allocate The memory to store File contents into.\n");
		return;
	}

	DWORD BytesRead = 0;
	if (ReadFile(FileHandle, FileInfo->data, FileSize.LowPart, &BytesRead, NULL) == FALSE)
	{
		Error = GetLastError();
		LogErrori("When Reading contents of a file got error: ", Error);
	}
	else
	{
		if (BytesRead != FileSize.LowPart)
		{
			Assert("Bytes read when reading entire file don't equal the file size!!" == 0)
		}
	}

	if (CloseHandle(FileHandle) == FALSE)
	{
		OutputDebugStringA("Couldn't close file handle at the end of Win32_ReadEntireFile function.\n");
	}
	FileInfo->size = FileSize.LowPart;
}

internal VOID Win32_SetupScreen(ScreenInfo *Screen, s32 Height, s32 Width, HINSTANCE Instance)
{
	Screen->Height = 1080;
	Screen->Width = 1920;

	Screen->WindowClass = { 0 };
	Screen->WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	Screen->WindowClass.lpfnWndProc = WindowProc;
	Screen->WindowClass.hInstance = Instance;
	Screen->WindowClass.hCursor = 0;
	Screen->WindowClass.lpszMenuName = "Window Menu";
	Screen->WindowClass.lpszClassName = "Window Class";
	if (!RegisterClassA(&Screen->WindowClass))
	{
		DWORD Error = GetLastError();
		LogErrori("When Registering WindowClass in Win32_SetupScreen got error: ", Error);
	}

	//@NOTE: WindowProc function needs to be fully operational before calling CreateWindow(Ex)(A/W) Because it calls it and uses it's return to create the window.
	//It can be set to return DefWindowProc(A/W) to return all default values and not have to handle them
	if ((Screen->WindowHandle = CreateWindowExA(0L, Screen->WindowClass.lpszClassName, "Win 32 Platform", WS_OVERLAPPED | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, Screen->Width, Screen->Height, 0, 0, Instance, 0)) == nullptr)
	{
		DWORD Error = GetLastError();
		LogErrori("When Retrieving a WindowHandle in Win32_SetupScreen got error: ", Error);
	}

	//@TODO Make this error path more @ROBUST. SetCapture actually returns an Handle to the previous Window owning mouse capture
	//If that window wasn't me, it will just post errors without meaning, even if stuff worked.
	if (SetCapture(Screen->WindowHandle) == NULL)
	{
		DWORD Error = GetLastError();
		LogErrori("When Setting Mouse Capture in Win32_SetupScreen got error: ", Error);
	}

	if (SetCursorPos(Screen->Width / 2, Screen->Height / 2) == NULL)
	{
		DWORD Error = GetLastError();
		LogErrori("When Setting Cursor Position in Win32_SetupScreen got error: ", Error);
	}
}

#if 0
internal VOID Win32_SetupOpenGLRenderingContext(ScreenInfo *Screen)
{
	PIXELFORMATDESCRIPTOR PixelFormat = {};

	PixelFormat.nSize			= sizeof(PIXELFORMATDESCRIPTOR);
	PixelFormat.nVersion		= 1;
	PixelFormat.dwFlags			= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	PixelFormat.iPixelType		= PFD_TYPE_RGBA;
	PixelFormat.cColorBits		= 32;	//Colordepth of the framebuffer
	PixelFormat.cDepthBits		= 24;	//Number of bits for the depthbuffer
	PixelFormat.cStencilBits	= 8;	//Number of bits for the stencilbuffer

	Screen->DeviceContext		= GetDC(Screen->WindowHandle);
	s32 PixelFormatValue		= ChoosePixelFormat(Screen->DeviceContext, &PixelFormat);
	SetPixelFormat(Screen->DeviceContext, PixelFormatValue, &PixelFormat);

	Screen->RenderingContext	= wglCreateContext(Screen->DeviceContext);
	wglMakeCurrent(Screen->DeviceContext, Screen->RenderingContext);
	LoadGLFunc(Screen->DeviceContext);

	glEnable(GL_DEPTH_TEST);
}
#endif

internal VOID Win32_ProcessKeyboard(KeyboardManager *Keyboard, MSG Msg)
{
	switch (Msg.message)
	{
		case WM_KEYDOWN:
			switch (Msg.wParam)
			{
			case VK_ESCAPE:
				Keyboard->isQuitting = TRUE;
				break;
			case VK_SHIFT:
				Keyboard->Key_Shift = TRUE;
				break;
			case VK_CONTROL:
				Keyboard->Key_Ctrl = TRUE;
				break;
			case VK_DOWN:
				Keyboard->DownArrow = TRUE;
				break;
			case VK_UP:
				Keyboard->UpArrow = TRUE;
				break;
			case VK_LEFT:
				Keyboard->LeftArrow = TRUE;
				break;
			case VK_RIGHT:
				Keyboard->RightArrow = TRUE;
				break;

			case 'W':
				Keyboard->Key_W = TRUE;
				break;
			case 'A':
				Keyboard->Key_A = TRUE;
				break;
			case 'S':
				Keyboard->Key_S = TRUE;
				break;
			case 'D':
				Keyboard->Key_D = TRUE;
				break;
			
			case 'P':
				Keyboard->Key_P = TRUE;
				break;
			}
			break;
		case WM_KEYUP:
			switch (Msg.wParam)
			{
			case VK_SHIFT:
				Keyboard->Key_Shift = FALSE;
				break;
			case VK_CONTROL:
				Keyboard->Key_Ctrl = FALSE;
				break;
			case VK_DOWN:
				Keyboard->DownArrow = FALSE;
				break;
			case VK_UP:
				Keyboard->UpArrow = FALSE;
				break;
			case VK_LEFT:
				Keyboard->LeftArrow = FALSE;
				break;
			case VK_RIGHT:
				Keyboard->RightArrow = FALSE;
				break;

			case 'W':
				Keyboard->Key_W = FALSE;
				break;
			case 'A':
				Keyboard->Key_A = FALSE;
				break;
			case 'S':
				Keyboard->Key_S = FALSE;
				break;
			case 'D':
				Keyboard->Key_D = FALSE;
				break;

			case 'P':
				Keyboard->Key_P = FALSE;
				break;
			}
		default:
			break;
	}
}

internal VOID Win32_ProcessMouse(MouseManager *Mouse, MSG Msg)
{
	POINT MousePos = {};
	switch (Msg.message)
	{
		case WM_MOUSEMOVE:
			if (!GetCursorPos(&MousePos))
			{
				DWORD Error = GetLastError();
				LogErrori("When retrieving mouse position in Win32_ProcessMouse got error: ", Error);
			}
			Mouse->mouseX = (f32)MousePos.x;
			Mouse->mouseY = (f32)MousePos.y;
			//Mouse->mouseX = (((f32)MousePos.x * 2) / 1920.0f) - 1.0f;
			//Mouse->mouseY = (((f32)MousePos.y * 2) / 1080.0f) - 1.0f;
			break;
		case WM_KEYDOWN:
			switch (Msg.wParam)
			{
				case VK_LBUTTON:
					Mouse->LeftDown = TRUE;
					break;
				case VK_RBUTTON:
					Mouse->RigthDown = TRUE;
					break;
				case VK_MBUTTON:
					Mouse->MiddleDown = TRUE;
					break;
			}
			break;
		case WM_KEYUP:
			switch (Msg.wParam)
			{
				case VK_LBUTTON:
					Mouse->LeftDown = FALSE;
					break;
				case VK_RBUTTON:
					Mouse->RigthDown = FALSE;
					break;
				case VK_MBUTTON:
					Mouse->MiddleDown = FALSE;
					break;
			}
			break;
	}
}

internal LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (Message)
	{
		case WM_ACTIVATEAPP:
			if (wParam == TRUE)
			{
				OutputDebugStringA("Activate App\n"); 
				//@TODO Make this error path more @ROBUST. SetCapture actually returns an Handle to the previous Window owning mouse capture
				//If that window wasn't me, it will just post errors without meaning, even if stuff worked.
				if (SetCapture(Window) == NULL)
				{
					DWORD Error = GetLastError();
					LogErrori("When Setting Mouse Capture in WindowProc got error: ", Error); OutputDebugStringA("\n");
				}
			}
			else
			{
				OutputDebugStringA("Deactivate App\n");
				if (ReleaseCapture() == NULL)
				{
					DWORD Error = GetLastError();
					LogErrori("When Setting Mouse Capture in WindowProc got error: ", Error); OutputDebugStringA("\n");
				}
			}
			break;

		default:
			return DefWindowProcA(Window, Message, wParam, lParam);
	}

	return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LARGE_INTEGER QueryFreq = {};
	if (!QueryPerformanceFrequency(&QueryFreq))
	{
		DWORD Error = GetLastError();
		LogErrori("Error in QueryPerformanceFrequency call: ", Error);
	}

	ScreenInfo Screen = {};
	Win32_SetupScreen(&Screen, 1920, 1080, hInstance);
	//Win32_SetupOpenGLRenderingContext(&Screen);

	MemoryArena Arena = {};
	Win32_AllocateMemory(&Arena.PermanentMemory, Gigabyte(1), 4);
	Arena.Alloc = &Win32_GiveMemory;
	Arena.Time = &Win32_Time;
	Arena.ReadTextFile = &Win32_ReadTextFile;
	Arena.ReadEntireFile = &Win32_ReadEntireFile;
	Arena.StartingTime = Win32_Time();
	Arena.MsPerFrame = 16.67f;

	InputManager Input = {};
	KeyboardManager Keyboard = {};
	MouseManager Mouse = {};
	Mouse.MouseCenterX = (f32)Screen.Width / 2.0f;
	Mouse.MouseCenterY = (f32)Screen.Height / 2.0f;
	Input.Keyboard	= &Keyboard;
	Input.Mouse		= &Mouse;

	Matroska MkvStruct = {};
	b32 Running = TRUE;

	LARGE_INTEGER FirstCounter = {};
	if (!QueryPerformanceCounter(&FirstCounter))
	{
		DWORD Error = GetLastError();
		LogErrori("Error calling QueryPerformanceCounter at the beginning of the loop: ", Error);
	}
	while(Running)
	{
		MSG Msg;
		while (PeekMessageA(&Msg, Screen.WindowHandle, 0, 0, PM_REMOVE))
		{
			Win32_ProcessKeyboard(&Keyboard, Msg);
			Win32_ProcessMouse(&Mouse, Msg);

			TranslateMessage(&Msg);
			DispatchMessageW(&Msg);

			if (Keyboard.isQuitting)
			{
				Running = FALSE;
			}
		}

		EditorLoop(&Arena, &MkvStruct);

#if 0
		HWND ActiveWindow = GetActiveWindow();
		if (ActiveWindow == Screen.WindowHandle)
		{
			SetCursorPos(Screen.Width / 2, Screen.Height / 2);
		}

		if (SwapBuffers(Screen.DeviceContext) == FALSE)
		{
			DWORD Error = GetLastError();
			LogErrori("In swapping buffers error: ", Error);
		}
#endif
		LARGE_INTEGER LastCounter = {};
		if (!QueryPerformanceCounter(&LastCounter))
		{
			DWORD Error = GetLastError();
			LogErrori("Error calling QueryPerformanceCounter at the end of the loop: ", Error);
		}

		s64 CounterDelta = LastCounter.QuadPart - FirstCounter.QuadPart;
		f32 MillisecondsPerFrame = ((f32)(1000 * CounterDelta) / (f32)QueryFreq.QuadPart);
		if (MillisecondsPerFrame < 16.67f)
		{
			DWORD RemainingTime = DWORD(16.67f - MillisecondsPerFrame);
			Sleep(RemainingTime);
		}

		FirstCounter = LastCounter;
	}
	return 0;
}