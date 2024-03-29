// main.c : Defines the entry point for the application.
//
// Windows Header Files:
#include <windows.h>
#include <psapi.h>

// C RunTime Header Files
#include <tchar.h>

#define MAX_LOADSTRING 100
#define WINDOW_CLASS_NAME _T("WinSwitcherHiddenWindow")

// Global Variables:
HINSTANCE hInst;		// current instance
HWND hWnd;		// main window handle

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(VOID);
VOID InitInstance(VOID);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID SwitchWindow(VOID);

int main(void)
{
	HINSTANCE hInst = GetModuleHandle(NULL);

	MSG msg;
	MyRegisterClass();

	// Perform application initialization:
	InitInstance ();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ExitProcess(msg.wParam);
}

ATOM MyRegisterClass(VOID)
{
	WNDCLASSEX wcex;
	wcex.cbSize        = sizeof(WNDCLASSEX);

	wcex.style         = 0;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInst;
	wcex.hIcon         = NULL;
	wcex.hCursor       = NULL;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	wcex.hIconSm       = NULL;

	return RegisterClassEx(&wcex);
}


VOID InitInstance(VOID)
{
	hWnd = CreateWindowEx(0, WINDOW_CLASS_NAME, _T(""), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		// Alt + tilde
		RegisterHotKey(hWnd, 1, 1, 192);
		break;
	case WM_DESTROY:
		UnregisterHotKey(hWnd, 1);
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		SwitchWindow();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// == REAL CODE AFTER THIS LINE ==

#define WS_GOODWINDOW (WS_VISIBLE | WS_CAPTION)

HWND hCurrentWindow;
DWORD nCurrentProcessId;
TCHAR szCurrentProcessName[MAX_PATH];

HWND hHighestRightWindow = NULL;
HWND hHighestWindow = NULL;

// compares two TCHAR strings
int _tstrcmp(TCHAR* sz1, TCHAR* sz2)
{
	int _iter = 0, ret;
	
	while (!(ret = sz1[_iter] - sz2[_iter]) && sz1[_iter]) ++_iter;

	if (ret < 0)
		return -1;
	else if (ret > 0)
		return 1;
	else
		return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	LONG lStyle;
	DWORD nProcessId;
	HANDLE hProcess;
	TCHAR szProcessName[MAX_PATH];

	// if it is bad window then skip
	lStyle = GetWindowLong(hWnd, GWL_STYLE);
	if ((lStyle & WS_GOODWINDOW) != WS_GOODWINDOW) return TRUE;

	GetWindowThreadProcessId(hWnd, &nProcessId);
	// if it is not from the same process
	if (nProcessId != nCurrentProcessId)
	{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, nProcessId);
		GetProcessImageFileName(hProcess, szProcessName, MAX_PATH);
		CloseHandle(hProcess);

		// if wrong process then skip
		if (_tstrcmp(szProcessName, szCurrentProcessName))
			return TRUE;
	}

	// if it is active then skip
	if (hWnd == hCurrentWindow) return TRUE;

	// let's find window with the biggest hWnd (hHighestWindow)
	//    and the window with the biggest hWnd, but not bigger than current (hHighestRightWindow)
	if (hWnd > hHighestRightWindow)
	{
		if (hWnd < hCurrentWindow)
			hHighestRightWindow = hWnd;
		hHighestWindow = hWnd;
	}

	return TRUE;
}

VOID SwitchWindow(VOID)
{
	HANDLE hCurrentProcess;

	// get information about current active window
	hCurrentWindow = GetForegroundWindow();
	GetWindowThreadProcessId(hCurrentWindow, &nCurrentProcessId);
	hCurrentProcess = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, nCurrentProcessId);
	GetProcessImageFileName(hCurrentProcess, szCurrentProcessName, MAX_PATH);
	CloseHandle(hCurrentProcess);

	// Enumerate all windows
	hHighestRightWindow = NULL;
	hHighestWindow = NULL;
	EnumWindows(&EnumWindowsProc, (LPARAM)0);

	if (!hHighestRightWindow) hHighestRightWindow = hHighestWindow;
	if (hHighestWindow) SetForegroundWindow(hHighestWindow);
}
