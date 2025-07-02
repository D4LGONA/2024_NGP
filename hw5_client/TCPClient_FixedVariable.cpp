#include "..\Common.h"
#include <iostream>
#include <cstdio>
using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <Server IP> <File Name>" << endl;
        return 1;
    }

    char* SERVERIP = argv[1];    // 명령행 인수로 IP 주소 받기
    string filename = argv[2];   // 명령행 인수로 파일이름 받기

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

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 서버 주소 설정
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr); // 명령행 인수로 받은 IP 주소 사용
    serveraddr.sin_port = htons(SERVERPORT);
    int retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
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

    // 자원 정리
    fclose(file);
    closesocket(sock);
    WSACleanup();
}
