#include "..\Common.h"
#include <windows.h>
#include <iostream>
#include <CommCtrl.h>
#include "resource.h"
#pragma comment(lib, "comctl32.lib")

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ClientMain(LPVOID arg);      // 소켓 통신 스레드 함수
DWORD WINAPI UpdateProgressBar(LPVOID arg); // 프로그레스 바 업데이트 스레드

SOCKET sock;
char buf[BUFSIZE + 1];        // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent;  // 이벤트
HWND hSendButton, hEdit1, hEdit2, hProgressBar, g_hDlg;
volatile int g_progress = 0;  // 송신률 저장 전역 변수
CRITICAL_SECTION csProgress;  // 동기화를 위한 크리티컬 섹션
HANDLE hUpdateThread;         // UI 업데이트 스레드 핸들
bool g_running = true;        // UI 스레드 실행 플래그

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection(&csProgress);

    // UI 업데이트 스레드 시작
    hUpdateThread = CreateThread(NULL, 0, UpdateProgressBar, NULL, 0, NULL);

    // 소켓 통신 스레드 시작
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

    // 대화상자 생성 및 메시지 루프
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    g_running = false;
    WaitForSingleObject(hUpdateThread, INFINITE);
    CloseHandle(hUpdateThread);
    DeleteCriticalSection(&csProgress);

    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);
    WSACleanup();
    return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    g_hDlg = hDlg;
    switch (uMsg) {
    case WM_INITDIALOG:
        hSendButton = GetDlgItem(hDlg, IDC_BUTTON2);
        hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
        hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS1);
        SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
        SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON1: { // 찾아보기 버튼
            OPENFILENAME ofn;
            wchar_t szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
            ofn.lpstrFilter = L"All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn) == TRUE) {
                SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT1), szFile);
            }
            break;
        }
        case IDC_BUTTON2: // 전송 버튼
            EnableWindow(hSendButton, FALSE);
            SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(hProgressBar, PBM_SETPOS, 0, 0);  // 초기 위치 0%
            WaitForSingleObject(hReadEvent, INFINITE);
            GetDlgItemTextA(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
            SetEvent(hWriteEvent);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            closesocket(sock);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}



DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    while (1) {
        // 파일 선택 대기 (전송 버튼이 눌렸을 때 hWriteEvent가 설정됨)
        WaitForSingleObject(hWriteEvent, INFINITE);

        // 에디트 박스에서 파일 경로 읽기
        wchar_t buf[BUFSIZE + 1] = { 0 };
        GetDlgItemTextW(g_hDlg, IDC_EDIT1, buf, BUFSIZE + 1);

        // 파일이 없을 때
        if (wcslen(buf) == 0) {
            MessageBox((HWND)arg, L"파일 경로가 비어 있습니다.", L"에러", MB_OK);
            EnableWindow(hSendButton, TRUE);
            SetEvent(hReadEvent);
            continue;
        }

        // 파일 경로에서 파일 이름 추출
        std::wstring filePath = buf;
        size_t lastSlashPos = filePath.find_last_of(L"\\/");
        std::wstring w_fileName = (lastSlashPos != std::wstring::npos) ? filePath.substr(lastSlashPos + 1) : filePath;
        std::string fileName(w_fileName.begin(), w_fileName.end());


        // 파일 열기
        FILE* file = _wfopen(filePath.c_str(), L"rb");
        if (!file) {
            MessageBox((HWND)arg, L"파일을 열 수 없습니다.", L"에러", MB_OK);
            EnableWindow(hSendButton, TRUE);
            SetEvent(hReadEvent);
            continue;
        }

        // 파일 크기 계산
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // 서버와 데이터 통신
        // 파일이름의 길이 전송하기
        size_t namesize = fileName.size();
        retval = send(sock, (char*)&namesize, sizeof(size_t), 0);
        if (retval == SOCKET_ERROR) err_display("send()");

        // 파일이름 전송하기
        retval = send(sock, fileName.c_str(), namesize, 0);
        if (retval == SOCKET_ERROR) err_display("send()");

        // 파일 크기 전송
        retval = send(sock, (char*)&fileSize, sizeof(size_t), 0);
        if (retval == SOCKET_ERROR) err_display("send()");

        // 파일 내용 전송
        char fileBuffer[BUFSIZE];
        long totalSent = 0;
        while (!feof(file)) {
            size_t bytes_read = fread(fileBuffer, 1, BUFSIZE, file);
            if (bytes_read > 0) {
                retval = send(sock, fileBuffer, bytes_read, 0);
                if (retval == SOCKET_ERROR) {
                    break;
                }
                totalSent += retval;

                EnterCriticalSection(&csProgress);
                g_progress = static_cast<int>((totalSent * 100) / fileSize);
                LeaveCriticalSection(&csProgress);
            }
        }

        fclose(file); // 파일 닫기

        std::wstring message = L"Client: [" + w_fileName + L"]" + L" 전송 완료!";

        // 리스트박스에 항목 추가
        SendMessage(GetDlgItem(g_hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)message.c_str());

        // 서버가 제대로 수신했는지
        bool fileReceived;
        retval = recv(sock, (char*)&fileReceived, sizeof(bool), MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
        }
        else if (fileReceived) {
            message = L"Server: [" + w_fileName + L"] 수신 완료!";
            SendMessage(GetDlgItem(g_hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)message.c_str());
        }

        EnableWindow(hSendButton, TRUE);
        SetEvent(hReadEvent);
    }
    return 0;
}

DWORD WINAPI UpdateProgressBar(LPVOID arg)
{
    while (g_running) {
        EnterCriticalSection(&csProgress);
        int currentProgress = g_progress;
        LeaveCriticalSection(&csProgress);

        SendMessage(hProgressBar, PBM_SETPOS, currentProgress, 0);
    }
    return 0;
}
