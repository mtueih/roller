#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define HOTKEY_ID 1
#define TIMER_ID 1
#define TICK_INTERVAL_MS 100

static HWND status_window;
static HWND status_label;
static int rolling = 0;
static int next_direction = 1; /* 1 = down, -1 = up */

static void update_status(void) {
	if (status_label) {
		SetWindowTextW(status_label, rolling
			? L"状态：运行中\r\nF8：暂停"
			: L"状态：已停止\r\nF8：启动");
	}
}

static void toggle_rolling(void) {
	rolling = !rolling;
	if (rolling) {
		next_direction = 1;
		SetTimer(status_window, TIMER_ID, TICK_INTERVAL_MS, NULL);
	} else {
		KillTimer(status_window, TIMER_ID);
	}
	update_status();
}

static void do_scroll(void) {
	const int steps = 3 + rand() % 4;
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	input.mi.mouseData = (DWORD)(next_direction * steps * WHEEL_DELTA);
	SendInput(1, &input, sizeof(input));
	next_direction = -next_direction;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_HOTKEY:
		if (wparam == HOTKEY_ID) toggle_rolling();
		return 0;
	case WM_TIMER:
		if (wparam == TIMER_ID && rolling) do_scroll();
		return 0;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		UnregisterHotKey(hwnd, HOTKEY_ID);
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProcW(hwnd, message, wparam, lparam);
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show_command) {
	(void)previous;
	(void)command_line;
	(void)show_command;
	srand((unsigned int)time(NULL));

	const wchar_t class_name[] = L"RollerStatusWindow";
	WNDCLASSW wc = { 0 };
	wc.lpfnWndProc = window_proc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = class_name;
	if (!RegisterClassW(&wc)) return 1;

	status_window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		class_name, L"滚轮模拟器", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 280, 120, NULL, NULL, instance, NULL);
	if (!status_window) return 1;

	status_label = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,
		12, 12, 240, 52, status_window, NULL, instance, NULL);
	if (!status_label) {
		DestroyWindow(status_window);
		return 1;
	}
	update_status();
	if (!RegisterHotKey(status_window, HOTKEY_ID, MOD_NOREPEAT, VK_F8)) {
		MessageBoxW(status_window, L"无法注册快捷键 F8。可能已被其他程序占用。", L"滚轮模拟器", MB_OK | MB_ICONERROR);
		DestroyWindow(status_window);
		return 1;
	}

	ShowWindow(status_window, SW_SHOW);
	UpdateWindow(status_window);
	MSG message;
	while (GetMessageW(&message, NULL, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
	return (int)message.wParam;
}
