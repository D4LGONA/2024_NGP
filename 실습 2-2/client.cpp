#include "..\Common.h"
#include "protocol.h"
#include <iostream>
using namespace std;

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	// connect
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT);
	connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	string s;
	char buf[256];
	while (true)
	{
		ZeroMemory(buf, BUFSIZE);
		cin >> buf;
		if (buf[0] == '\0') break;

		send(sock, buf, BUFSIZE, 0);

		recv(sock, buf, BUFSIZE, 0);
		cout << buf << endl;
	}

	closesocket(sock);
	WSACleanup();


}