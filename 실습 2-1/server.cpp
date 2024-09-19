//#include "..\Common.h"
//#include "protocol.h"
//#include <iostream>
//using namespace std;
//
//int main(int argc, char* argv[])
//{
//	// 윈속 초기화
//	WSADATA wsa;
//	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//		return 1;
//	printf("[알림] 윈속 초기화 성공\n");
//
//	// 소켓 생성
//	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//	if (sock == INVALID_SOCKET) err_quit("socket()");
//	printf("[알림] 소켓 생성 성공\n");
//
//	// startup - bind - listen - accept - send/recv - close
//
//	// sockaddr과 sockaddr_in의 차이?
//	// 인터넷 연결을 할 때는 sockaddr_in을 사용한다
//	// https://cafe.daum.net/krtg/Fntr/9?q=D_M.JM.ytBUzQ0&
//
//	sockaddr_in server_addr;
//	ZeroMemory(&server_addr, sizeof(server_addr));
//	server_addr.sin_family = AF_INET;
//	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//	server_addr.sin_port = htons(PORT);
//	bind(sock, (sockaddr*)&server_addr, sizeof(server_addr));
//
//	// listen
//	listen(sock, SOMAXCONN);
//
//	// accept
//	SOCKET client_sock;
//	sockaddr_in client_addr;
//	ZeroMemory(&client_addr, sizeof(client_addr));
//	int n = sizeof(client_addr);
//	char buf[BUFSIZE];
//
//	while(true)
//	{
//		// 왜 sizeof를 줘야 하는지 모르겟넹
//		client_sock = accept(sock, (sockaddr*)&client_addr, &n); 
//
//		while (true)
//		{
//			auto ret = recv(client_sock, buf, BUFSIZE, 0);
//			if (ret == 0) break;
//			cout << buf << endl;
//
//			send(client_sock, buf, BUFSIZE, 0);
//		}
//
//		closesocket(client_sock);
//	}
//
//	// 소켓 닫기
//	closesocket(sock);
//
//	// 윈속 종료
//	WSACleanup();
//	return 0;
//}