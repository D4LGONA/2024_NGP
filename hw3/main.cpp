#include "..\Common.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "도메인을 제대로 입력하세요!" << endl;
        //return 1;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed with error: " << result << endl;
        return 1;
    }

    // 입력받은 도메인 이름
    string s;
    cin >> s;
    const char* domain_name = s.c_str();

    // domain_name으로 hostent 구조체 가져오기
    struct hostent* ptr = gethostbyname(domain_name);

    // h_aliases 출력
    for (int i = 0; ptr->h_aliases[i] != nullptr; ++i) {
        cout << "Alias " << i + 1 << ": " << ptr->h_aliases[i] << endl;
    }

    /*
    gethostbyname 함수에서 반환된 호스트 구조체의 메모리는 스레드 로컬 스토리지의 Winsock DLL에 의해 내부적으로 할당됩니다.
    스레드에서 gethostbyaddr 또는 gethostbyname 함수가 호출되는 횟수에 관계없이 단일 호스트 구조만 할당되고 사용됩니다. 
    동일한 스레드의 gethostbyname 또는 gethostbyaddr 함수에 대한 추가 호출을 수행해야 하는 경우 
    반환된 호스트 구조체를 애플리케이션 버퍼에 복사해야 합니다. 
    그렇지 않으면 반환 값은 동일한 스레드에서 후속 gethostbyname 또는 gethostbyaddr 호출에 의해 덮어씁니다. 
    반환된 호스트 구조에 할당된 내부 메모리는 스레드가 종료될 때 Winsock DLL에 의해 해제됩니다
    */

    for (int i = 0; ptr->h_aliases[i] != nullptr; ++i) {
        cout << "Alias " << i + 1 << ": " << ptr->h_aliases[i] << endl;
        struct hostent* tmpptr = gethostbyname(ptr->h_aliases[i]);
        for (int j = 0; tmpptr->h_addr_list[j] != nullptr; ++j) {
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpptr->h_addr_list[j], ip_str, sizeof(ip_str));
            cout << "Alias" << i + 1 << "의 IP Address " << j + 1 << ": " << ip_str << endl;
        }
    }

    // h_addr_list 출력
    for (int i = 0; ptr->h_addr_list[i] != nullptr; ++i) {
        char ip_str[INET_ADDRSTRLEN]; 
        inet_ntop(AF_INET, ptr->h_addr_list[i], ip_str, sizeof(ip_str));
        cout << "IP Address " << i + 1 << ": " << ip_str << endl;
    }

    WSACleanup();
}
