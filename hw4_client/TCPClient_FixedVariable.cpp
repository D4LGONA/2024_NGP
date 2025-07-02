#include "..\Common.h"
#include <iostream>
#include <cstdio>
using namespace std;

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
    int retval;
    string filename;
    if (argc > 1) filename = argv[1]; // 명령행 인수로 파일이름 받기

    // C 스타일로 파일 열기
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        cerr << "File not found: " << filename << endl;
        return 1;
    }

    // 파일 크기 계산
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE]{};

    // 서버와 데이터 통신
    // 파일이름의 길이 전송하기
    size_t namesize = filename.size();
    retval = send(sock, (char*)&namesize, sizeof(size_t), 0);
    if (retval == SOCKET_ERROR) err_display("send()");

    // 파일이름 전송하기
    retval = send(sock, filename.c_str(), namesize, 0);
    if (retval == SOCKET_ERROR) err_display("send()");

    // 파일 크기 전송
    retval = send(sock, (char*)&filesize, sizeof(size_t), 0);
    if (retval == SOCKET_ERROR) err_display("send()");

    // 파일내용 전송하기
    while (!feof(file)) {
        size_t bytes_read = fread(buf, 1, BUFSIZE, file);
        if (bytes_read > 0) {
            retval = send(sock, buf, bytes_read, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                break;
            }
        }
    }

    fclose(file);
    closesocket(sock);
    WSACleanup();
}
