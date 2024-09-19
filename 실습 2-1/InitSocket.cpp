#include "..\Common.h"
#include <iostream>

int main(int argc, char* argv[])
{
	using namespace std;
	// ���� �ʱ�ȭ
	WSADATA wsa;
	char arr[2] = { 2, 2 };
	if (WSAStartup(WORD(arr), &wsa) != 0)
		return 1;
	printf("[�˸�] ���� �ʱ�ȭ ����\n");
	// (1.1 ver) 257, 514, WinSock 2.0, Running
	// (2.2 ver) 514, 514, WinSock 2.0, Running
	cout << wsa.wVersion << ", " << wsa.wHighVersion << ", " << wsa.szDescription << ", " << wsa.szSystemStatus << endl;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[�˸�] ���� ���� ����\n");

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}