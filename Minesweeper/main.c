#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// easy		9x9		10 mines
// medium	16x16	40 mines
// hard		30x16	99 mines

#define MINE_COUNT 40

#define TILES_X 16
#define TILES_Y 16

#define SPRITE_WIDTH 32
#define SPRITE_HEIGHT 32

#define WINDOW_WIDTH SPRITE_WIDTH * TILES_X
#define WINDOW_HEIGHT SPRITE_HEIGHT * TILES_Y

int minefield[TILES_X * TILES_Y] = { 0 };
int minefieldClicked[TILES_X * TILES_Y] = { 0 };
int minefieldFlagged[TILES_X * TILES_Y] = { 0 };

HANDLE imageHandle;

struct MPOINT {
	int x;
	int y;
};

struct MPOINT Convert1dTo2dIndex(int i) {
	struct MPOINT p = { 0 };
	p.x = i % TILES_X;
	p.y = i / TILES_X;
	return p;
};

int Convert2dTo1dIndex(int x, int y) {
	if (x < 0 || x > TILES_X - 1 || y < 0 || y > TILES_Y - 1) {
		return -1;
	}
	return (y * TILES_X) + x;
}

const struct MPOINT OPEN_EMPTY = { 0, 32 };
const struct MPOINT OPEN_ONE = { 32, 32 };
const struct MPOINT OPEN_TWO = { 64, 32 };
const struct MPOINT OPEN_THREE = { 96, 32 };
const struct MPOINT OPEN_FOUR = { 128, 32 };
const struct MPOINT OPEN_FIVE = { 160, 32 };
const struct MPOINT OPEN_SIX = { 192, 32 };
const struct MPOINT OPEN_SEVEN = { 224, 32 };
const struct MPOINT OPEN_EIGHT = { 256, 32 };

const struct MPOINT OPEN_MINE_CLEARED = { 64, 64 };
const struct MPOINT OPEN_MINE_DETONATED = { 96, 64 };

const struct MPOINT HIDDEN_BUTTON = { 0, 64 };
const struct MPOINT HIDDEN_FLAG = { 32, 64 };

// forward declaration of MainWndProc
LRESULT MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// forward declaration of OutputLastError
void OutputLastError();

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

	// define window class for main window
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = MainWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = NULL;
	wndClass.hCursor = LoadCursor(NULL, IDC_HAND);
	wndClass.hbrBackground = NULL;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("MainWindowClass");
	wndClass.hIconSm = NULL;

	// register window class
	ATOM wndClassAtom = RegisterClassEx(&wndClass);

	// check if window class registration failed
	if (wndClassAtom == NULL) {
		OutputLastError();
		return 1;
	}

	HWND hWnd = CreateWindowEx(
		NULL,
		TEXT("MainWindowClass"),
		TEXT("Minesweeper"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	// check if window creation failed
	if (hWnd == NULL) {
		OutputLastError();
		return 1;
	}

	// show the window
	ShowWindow(hWnd, nCmdShow);

	// update the window
	BOOL updateStatus = UpdateWindow(hWnd);

	// check if window update failed
	if (updateStatus == NULL) {
		OutputLastError();
		return 1;
	}

	// specify the desired window size
	RECT adjustWindowRectangle = { 0 };
	adjustWindowRectangle.left = 0;
	adjustWindowRectangle.top = 0;
	adjustWindowRectangle.right = WINDOW_WIDTH;
	adjustWindowRectangle.bottom = WINDOW_HEIGHT;

	// calculate the window rect so that the client are matches the desired size
	BOOL adjustStatus = AdjustWindowRectEx(&adjustWindowRectangle, WS_OVERLAPPEDWINDOW, FALSE, NULL);

	// check if window adjustment failed
	if (adjustStatus == FALSE) {
		OutputLastError();
		return 1;
	}

	// get monitor handle
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	int windowCenterPositionX = 0;
	int windowCenterPositionY = 0;
	int windowAdjustedWidth = adjustWindowRectangle.right - adjustWindowRectangle.left;
	int windowAdjustedHeight = adjustWindowRectangle.bottom - adjustWindowRectangle.top;

	if (hMonitor != NULL) {

		MONITORINFO mInfo = { 0 };
		mInfo.cbSize = sizeof(MONITORINFO);

		// get monitor info
		BOOL miStatus = GetMonitorInfo(hMonitor, &mInfo);

		if (miStatus != FALSE) {
			windowCenterPositionX = (mInfo.rcWork.left + mInfo.rcWork.right) / 2 - windowAdjustedWidth / 2;
			windowCenterPositionY = (mInfo.rcWork.top + mInfo.rcWork.bottom) / 2 - windowAdjustedHeight / 2;
		}

	}

	// resize the window to the calculated size
	BOOL setStatus = SetWindowPos(
		hWnd,
		NULL,
		windowCenterPositionX,
		windowCenterPositionY,
		windowAdjustedWidth,
		windowAdjustedHeight,
		SWP_SHOWWINDOW
	);

	// check if set window position failed
	if (setStatus == FALSE) {
		OutputLastError();
		return 1;
	}

	imageHandle = LoadImage(NULL, TEXT("assets\\minesweeper.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (imageHandle == NULL) {
		OutputLastError();
		return 1;
	}

	// message struct for window messages
	MSG msg;
	// store return value of GetMessageA here
	BOOL bRet;
	// blocking message pump
	while ((bRet = GetMessage(&msg, hWnd, 0, 0)) != FALSE) {
		// if GetMessage returns error bRet is set to -1
		if (bRet == -1) {
			OutputLastError();
			return 1;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(imageHandle);

	return 0;

}

void enqueue(int value, int queue[], int queueLength) {

	queue[queueLength] = value;

}

int dequeue(int queue[], int queueLength) {

	if (queueLength == 0) return -1;

	int value = queue[0];

	for (int i = 1; i <= queueLength - 1; i++) {
		queue[i - 1] = queue[i];
	}

	return value;

}

void OMFC(int minefieldIndex) {

	int queue[TILES_X * TILES_Y] = { 0 };
	int queueLength = 0;

	enqueue(minefieldIndex, queue, queueLength);
	queueLength += 1;

	while (queueLength != 0) {

		int currentIndex = dequeue(queue, queueLength);
		queueLength -= 1;
		int currentValue = minefield[currentIndex];
		struct MPOINT p = Convert1dTo2dIndex(currentIndex);
		int cy = p.y;
		int cx = p.x;

		minefieldClicked[currentIndex] = 1;

		if (currentValue == 0) {

			int explore[8];

			explore[0] = Convert2dTo1dIndex(cx - 1, cy - 1);	// top left
			explore[1] = Convert2dTo1dIndex(cx + 0, cy - 1);	// top
			explore[2] = Convert2dTo1dIndex(cx + 1, cy - 1);	// top right
			explore[3] = Convert2dTo1dIndex(cx + 1, cy + 0);	// right
			explore[4] = Convert2dTo1dIndex(cx + 1, cy + 1);	// bottom right
			explore[5] = Convert2dTo1dIndex(cx + 0, cy + 1);	// bottom
			explore[6] = Convert2dTo1dIndex(cx - 1, cy + 1);	// bottom left
			explore[7] = Convert2dTo1dIndex(cx - 1, cy + 0);	// left

			for (int i = 0; i < 8; i++) {
				int nIndex = explore[i];
				if (nIndex < 0) continue;
				int nVisited = minefieldClicked[nIndex];
				int nFlagged = minefieldFlagged[nIndex];
				if (nVisited > 0 || nFlagged > 0) continue;
				minefieldClicked[nIndex] = 1;
				enqueue(nIndex, queue, queueLength);
				queueLength += 1;
			}

		}

	}


}

bool checkWin() {

	int cOpen = 0;
	int cFlag = 0;

	for (int y = 0; y < TILES_Y; y += 1) {
		for (int x = 0; x < TILES_X; x += 1) {

			int c = Convert2dTo1dIndex(x, y);

			if (minefieldClicked[c] == 1) cOpen += 1;
			if (minefieldFlagged[c] == 1) cFlag += 1;

		}
	}

	if (cFlag == MINE_COUNT && cOpen == (TILES_X * TILES_Y - MINE_COUNT)) {
		return true;
	}

	return false;

}

bool checkDefeat() {

	bool defeated = false;

	for (int y = 0; y < TILES_Y; y += 1) {
		for (int x = 0; x < TILES_X; x += 1) {

			int c = Convert2dTo1dIndex(x, y);

			if (minefieldClicked[c] == 1 && minefield[c] == 9) {
				minefield[c] = 10;
				defeated = true;
			}

		}
	}

	return defeated;
}

void openField() {
	for (int y = 0; y < TILES_Y; y += 1) {
		for (int x = 0; x < TILES_X; x += 1) {

			int c = Convert2dTo1dIndex(x, y);

			minefieldClicked[c] = 1;
			minefieldFlagged[c] = 0;

		}
	}
}

void initGame() {

	memset(minefield, 0, sizeof(minefield));
	memset(minefieldClicked, 0, sizeof(minefieldClicked));
	memset(minefieldFlagged, 0, sizeof(minefieldFlagged));

	srand(time(NULL));

	int prn = rand();

	int count = MINE_COUNT;
	while (count != 0) {
		prn = rand();
		prn = prn % (TILES_X * TILES_Y);
		int v = minefield[prn];
		if (v != 0) {
			continue;
		}
		minefield[prn] = 9;
		count--;
	}

	for (int y = 0; y < TILES_Y; y += 1) {
		for (int x = 0; x < TILES_X; x += 1) {

			int c = Convert2dTo1dIndex(x, y);
			int v = minefield[c];
			if (v != 9) continue;

			int ntl = Convert2dTo1dIndex(x - 1, y - 1);
			int ntt = Convert2dTo1dIndex(x + 0, y - 1);
			int ntr = Convert2dTo1dIndex(x + 1, y - 1);
			int nrr = Convert2dTo1dIndex(x + 1, y + 0);
			int nbr = Convert2dTo1dIndex(x + 1, y + 1);
			int nbb = Convert2dTo1dIndex(x + 0, y + 1);
			int nbl = Convert2dTo1dIndex(x - 1, y + 1);
			int nll = Convert2dTo1dIndex(x - 1, y + 0);

			if (ntl != -1 && minefield[ntl] != 9) minefield[ntl] += 1;
			if (ntt != -1 && minefield[ntt] != 9) minefield[ntt] += 1;
			if (ntr != -1 && minefield[ntr] != 9) minefield[ntr] += 1;
			if (nrr != -1 && minefield[nrr] != 9) minefield[nrr] += 1;
			if (nbr != -1 && minefield[nbr] != 9) minefield[nbr] += 1;
			if (nbb != -1 && minefield[nbb] != 9) minefield[nbb] += 1;
			if (nbl != -1 && minefield[nbl] != 9) minefield[nbl] += 1;
			if (nll != -1 && minefield[nll] != 9) minefield[nll] += 1;

		}
	}

}


LRESULT MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_CREATE:
	{
		initGame();
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		return 0;
	}
	case WM_CHAR:
	{
		if (wParam == 'n') {
			int answer = MessageBox(hWnd, TEXT("New game?"), TEXT("New game?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
			if (answer == IDYES) {
				initGame();
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
			}
		}
		else if (wParam == VK_ESCAPE) {
			ExitProcess(0);
		}
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT cr;
		HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));

		BITMAP bm;
		HBITMAP image = (HBITMAP)imageHandle;

		HDC hdc = BeginPaint(hWnd, &ps);
		HDC hdcMem = CreateCompatibleDC(hdc);

		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, image);

		GetObject(imageHandle, sizeof(bm), &bm);

		GetClientRect(hWnd, &cr);
		FillRect(hdc, &cr, brush);

		for (int y = 0; y < TILES_Y; y += 1) {
			for (int x = 0; x < TILES_X; x += 1) {
				struct MPOINT sprite;

				int i = Convert2dTo1dIndex(x, y);

				int m = minefield[i];
				int v = minefieldClicked[i];
				int f = minefieldFlagged[i];

				switch (m) {
				case 0:
					sprite = OPEN_EMPTY;
					break;
				case 1:
					sprite = OPEN_ONE;
					break;
				case 2:
					sprite = OPEN_TWO;
					break;
				case 3:
					sprite = OPEN_THREE;
					break;
				case 4:
					sprite = OPEN_FOUR;
					break;
				case 5:
					sprite = OPEN_FIVE;
					break;
				case 6:
					sprite = OPEN_SIX;
					break;
				case 7:
					sprite = OPEN_SEVEN;
					break;
				case 8:
					sprite = OPEN_EIGHT;
					break;
				case 9:
					sprite = OPEN_MINE_CLEARED;
					break;
				case 10:
					sprite = OPEN_MINE_DETONATED;
					break;
				default:
					sprite = OPEN_EMPTY;
					break;
				}

				if (v == 0) {
					sprite = HIDDEN_BUTTON;
				}

				if (f == 1) {
					sprite = HIDDEN_FLAG;
				}

				BitBlt(hdc, x * SPRITE_WIDTH, y * SPRITE_HEIGHT, SPRITE_WIDTH, SPRITE_HEIGHT, hdcMem, sprite.x, sprite.y, SRCCOPY);
			}
		}

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);

		BOOL epStatus = EndPaint(hWnd, &ps);

		DeleteObject(brush);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		WORD x = LOWORD(lParam);
		WORD y = HIWORD(lParam);
		x = x / SPRITE_WIDTH;
		y = y / SPRITE_HEIGHT;
		int i = Convert2dTo1dIndex(x, y);


		int f = minefieldFlagged[i];
		int alreadyClicked = minefieldClicked[i];

		if (f == 1) {
			OutputDebugString(TEXT("The clicked field is flagged, so the field cannot be opened.\n"));
		}
		else if (alreadyClicked == 1) {
			OutputDebugString(TEXT("The field is already open.\n"));
		}
		else {
			OMFC(i);

			bool defeat = checkDefeat();
			bool win = checkWin();
			if (win == true) {
				openField();
				OutputDebugString(TEXT("WINNER WINNER CHICKEN DINNER!!!\n"));
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				MessageBox(hWnd, TEXT("WINNER WINNER CHICKEN DINNER!!!"), TEXT("CONGRATULATIONS"), MB_ICONINFORMATION | MB_OK);
			}

			if (defeat == true) {
				openField();
				OutputDebugString(TEXT("BOOOOOOOOOOOOOOOOM!!!\n"));
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				MessageBox(hWnd, TEXT("BOOOOOOMMM!!!"), TEXT("GAME OVER"), MB_ICONERROR | MB_OK);
			}

			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
		}
		return 0;
	}
	case WM_RBUTTONUP:
	{
		WORD x = LOWORD(lParam);
		WORD y = HIWORD(lParam);
		x = x / SPRITE_WIDTH;
		y = y / SPRITE_HEIGHT;
		int i = Convert2dTo1dIndex(x, y);

		int alreadyClicked = minefieldClicked[i];

		if (alreadyClicked == 1) {
			OutputDebugString(TEXT("Cannot place flag, the clicked field was already opened.\n"));
		}
		else {
			int t = minefieldFlagged[i];
			if (t == 0) {
				minefieldFlagged[i] = 1;
			}
			else {
				minefieldFlagged[i] = 0;
			}

			bool win = checkWin();
			if (win == true) {
				openField();
				OutputDebugString(TEXT("WINNER WINNER CHICKEN DINNER!!!\n"));
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				MessageBox(hWnd, TEXT("WINNER WINNER CHICKEN DINNER!!!"), TEXT("CONGRATULATIONS"), MB_ICONINFORMATION | MB_OK);
			}

			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
		}
		return 0;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}