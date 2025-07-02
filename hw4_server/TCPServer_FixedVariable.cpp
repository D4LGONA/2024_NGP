#include "..\Common.h"
#include <iostream>
#include <cstdio>  // C 스타일 파일 입출력
#include <cstring>
using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char* argv[]) {
    int retval;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;

    while (true) {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        cout << "\n[TCP 서버] 클라이언트 접속: IP 주소=" << addr
            << ", 포트 번호=" << ntohs(clientaddr.sin_port) << endl;

        // 파일 이름 길이 수신
        size_t filename_len;
        retval = recv(client_sock, (char*)&filename_len, sizeof(size_t), MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }

        // 파일 이름 수신
        char* filename = new char[filename_len + 1];
        retval = recv(client_sock, filename, filename_len, MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            delete[] filename;
            break;
        }
        filename[filename_len] = '\0';

        // 파일 크기 수신
        size_t filesize;
        retval = recv(client_sock, (char*)&filesize, sizeof(size_t), MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            delete[] filename;
            break;
        }

        FILE* file = fopen(filename, "wb");
        if (!file) { // 파일이 열리지 않음
            delete[] filename;
            closesocket(client_sock);
            continue;
        }

        char buf[BUFSIZE]{};
        size_t received = 0;

        // 파일 내용 수신
        while (filesize > received) {
            retval = recv(client_sock, buf, min(BUFSIZE, filesize - received), MSG_WAITALL);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }

            fwrite(buf, 1, retval, file);
            received += retval;

            float progress = int((float(received) / filesize) * 10000) / 100.0;
            cout << "\r[파일 수신중] 전송률: " << progress << "%" << "       " << flush;
        }
        cout << endl;
        fclose(file);
        cout << "[TCP 서버] 파일 수신 및 저장 완료: " << filename << endl;
        delete[] filename;
        closesocket(client_sock);
    }

    closesocket(listen_sock);
    WSACleanup();
}
