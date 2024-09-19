#include "..\Common.h"
#include <iostream>

int main()
{
	using namespace std;
	WSADATA wsa;
	unsigned char arr[2] = { 2, 2 };
	WORD version;
	memcpy(&version, arr, sizeof(WORD));
	if (WSAStartup(version, &wsa) != 0)
		return 1;
	printf("[알림] 윈속 초기화 성공\n");

	cout << "wVersion: " << int(LOBYTE(wsa.wVersion)) << "." << int(HIBYTE(wsa.wVersion)) << endl;
	cout << "wHighVersion: " << int(LOBYTE(wsa.wHighVersion)) << "." << int(HIBYTE(wsa.wHighVersion)) << endl;
	cout << "szDescription: " << wsa.szDescription << endl;
	cout << "szSystemStatus: " << wsa.szSystemStatus <<endl;

	WSACleanup();
	return 0;
}