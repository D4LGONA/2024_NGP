#include <windows.h>
#include <thread>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RenderingThread(HWND hwnd);

HBITMAP hBufferBitmap;
HDC hBufferDC;
bool isRunning = true;

// 캐릭터 위치
int characterX = 100;
int characterY = 100;
const int moveStep = 5; // 이동 속도

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Rendering Thread Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    // 렌더링 스레드 시작
    std::thread renderThread(RenderingThread, hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    isRunning = false;
    renderThread.join();

    DeleteDC(hBufferDC);
    DeleteObject(hBufferBitmap);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        // 버퍼 초기화
        HDC hdc = GetDC(hwnd);
        hBufferDC = CreateCompatibleDC(hdc);
        RECT rect;
        GetClientRect(hwnd, &rect);
        hBufferBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        SelectObject(hBufferDC, hBufferBitmap);
        ReleaseDC(hwnd, hdc);
    }
    return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hBufferDC, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
    }
    return 0;
    case WM_KEYDOWN: // 키 입력 처리
        switch (wParam) {
        case 'W': characterY -= moveStep; break; // 위로 이동
        case 'S': characterY += moveStep; break; // 아래로 이동
        case 'A': characterX -= moveStep; break; // 왼쪽으로 이동
        case 'D': characterX += moveStep; break; // 오른쪽으로 이동
        }
        InvalidateRect(hwnd, NULL, FALSE); // 화면 갱신 요청
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void RenderingThread(HWND hwnd) {
    // 더블 버퍼링에 사용할 메모리 DC에서 작업
    while (isRunning) {
        RECT rect;
        GetClientRect(hwnd, &rect);

        // 배경 그리기
        HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30)); // 배경을 어두운 색으로 채우기
        FillRect(hBufferDC, &rect, hBrush);
        DeleteObject(hBrush);

        // 캐릭터 그리기 (단순한 사각형으로 표현)
        HBRUSH characterBrush = CreateSolidBrush(RGB(255, 0, 0)); // 캐릭터 색상: 빨간색
        RECT characterRect = { characterX, characterY, characterX + 20, characterY + 20 };
        FillRect(hBufferDC, &characterRect, characterBrush);
        DeleteObject(characterBrush);

        // 화면 갱신 요청
        InvalidateRect(hwnd, NULL, FALSE);

        // 프레임 속도 조절 (예: 60fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
