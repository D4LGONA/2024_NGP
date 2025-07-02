#include "../Common.h"
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <atomic>

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512

HANDLE hConsole;  // 콘솔 핸들
atomic_int clientCount = 0;  // 클라이언트 수를 카운트
CRITICAL_SECTION cs;  // 크리티컬 섹션 객체

// 콘솔 커서 위치 설정 함수
void SetCursorPosition(int row, int col) {
    COORD coord;
    coord.X = col;
    coord.Y = row;
    SetConsoleCursorPosition(hConsole, coord);
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
    int retval;
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    char addr[INET_ADDRSTRLEN];  // 클라이언트의 IP 주소를 저장할 배열
    int addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);

    // IP 주소 변환
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    int row = clientCount.fetch_add(1) * 3; // 클라이언트마다 줄 할당... atomic하게

    // 클라이언트 접속 정보 출력 (첫 번째 줄)
    EnterCriticalSection(&cs);  // 크리티컬 섹션 진입
    SetCursorPosition(row, 0);
    cout << "[TCP 서버] 클라이언트 접속: IP 주소=" << addr << ", 포트 번호=" << ntohs(clientaddr.sin_port) << "                    " << endl;
    LeaveCriticalSection(&cs);  // 크리티컬 섹션 탈출

    while(true) {
        // 파일 이름 길이 수신
        size_t filename_len;
        retval = recv(client_sock, (char*)&filename_len, sizeof(size_t), MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            cerr << "recv() failed" << endl;
            return -1;
        }

        // 파일 이름 수신
        char* filename = new char[filename_len + 1];
        retval = recv(client_sock, filename, filename_len, MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            cerr << "recv() failed" << endl;
            delete[] filename;
            return -1;
        }
        filename[filename_len] = '\0';

        // 파일 크기 수신
        size_t filesize;
        retval = recv(client_sock, (char*)&filesize, sizeof(size_t), MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            cerr << "recv() failed" << endl;
            delete[] filename;
            return -1;
        }

        FILE* file = fopen(filename, "wb");
        if (!file) {
            cerr << "Failed to open file" << endl;
            delete[] filename;
            closesocket(client_sock);
            return -1;
        }

        // 다시 보내기 할때 깔끔하게 하려고 추가
        EnterCriticalSection(&cs);  
        cout << string(80, ' ') << flush; 
        SetCursorPosition(row + 2, 0);  
        cout << string(80, ' ') << flush; 
        LeaveCriticalSection(&cs); 

        size_t received = 0;

        // 파일 내용 수신 (두 번째 줄에 전송률 출력)
        while (filesize > received) {
            retval = recv(client_sock, buf, min(BUFSIZE, filesize - received), MSG_WAITALL); // 0써도됨.
            if (retval == SOCKET_ERROR) {
                cerr << "recv() failed" << endl;
                break;
            }

            fwrite(buf, 1, retval, file);
            received += retval;

            float progress = int((float(received) / filesize) * 10000) / 100.0;

            // 전송률 출력 (두 번째 줄)
            EnterCriticalSection(&cs);  // 크리티컬 섹션 진입
            SetCursorPosition(row + 1, 0);
            cout << "[파일 수신중] 전송률: " << progress << "%" << "                    " << flush;
            LeaveCriticalSection(&cs);  // 크리티컬 섹션 탈출
        }

        fclose(file);

        // 파일 수신 완료 메시지 출력 (세 번째 줄)
        EnterCriticalSection(&cs);  // 크리티컬 섹션 진입
        SetCursorPosition(row + 2, 0);
        cout << "[TCP 서버] 파일 수신 및 저장 완료: " << filename << "                    " << endl;
        LeaveCriticalSection(&cs);  // 크리티컬 섹션 탈출

        bool fileReceived = true;
        retval = send(client_sock, (char*)&fileReceived, sizeof(bool), 0);
        if (retval == SOCKET_ERROR) {
            cerr << "send() failed" << endl;
        }
        delete[] filename;
    }

    closesocket(client_sock);
    return 0;
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 콘솔 핸들 가져오기
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 크리티컬 섹션 초기화
    InitializeCriticalSection(&cs);

    // 소켓 생성
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    HANDLE hThread;

    while (true) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            cerr << "accept() failed" << endl;
            break;
        }

        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) {
            closesocket(client_sock);
        }
        else {
            CloseHandle(hThread);
        }
    }

    // 소켓 닫기
    closesocket(listen_sock);

    // 크리티컬 섹션 삭제
    DeleteCriticalSection(&cs);

    // 윈속 종료
    WSACleanup();
    return 0;
}
