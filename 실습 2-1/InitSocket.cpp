#include "..\Common.h"
#include <iostream>

int main(int argc, char* argv[])
{
	using namespace std;
	// 윈속 초기화
	WSADATA wsa;
	char arr[2] = { 2, 2 };
	if (WSAStartup(WORD(arr), &wsa) != 0)
		return 1;
	printf("[알림] 윈속 초기화 성공\n");
	// (1.1 ver) 257, 514, WinSock 2.0, Running
	// (2.2 ver) 514, 514, WinSock 2.0, Running
	cout << wsa.wVersion << ", " << wsa.wHighVersion << ", " << wsa.szDescription << ", " << wsa.szSystemStatus << endl;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[알림] 소켓 생성 성공\n");

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}