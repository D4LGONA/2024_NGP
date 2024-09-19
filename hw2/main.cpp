#include "..\Common.h"
#include <iostream>

bool IsLittleEndian()
{
	unsigned short data = 1;
	auto data2 = htons(data);
	if (data2 == data) return false;
	else return true;
}

bool IsBidEndian()
{
	return not IsLittleEndian();
}

int main()
{
	using namespace std;

	if (IsLittleEndian())
		cout << "Little Endian" << endl;
	else
		cout << "Big Endian" << endl;
}