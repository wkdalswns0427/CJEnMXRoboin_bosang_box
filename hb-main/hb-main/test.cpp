#include <iostream>

using namespace std;

int main( )
{
	int nNum(14);

	//10진법 출력
	cout << "10진법 : " << nNum << endl;

	//8진법 모드로 설정
	cout << oct;
	cout << "08진법 : " << nNum << endl;

	//16진법 모드로 설정
	cout << hex;
	cout << "16진법 : " << nNum << endl;

	//10진법 다른 방법
	dec(cout);
	cout << "10진법 : " << nNum << endl;

	//8진법 다른 방법
	oct(cout);
	cout << "08진법 : " << nNum << endl;

	//16진법 다른 방법
	hex(cout);
	cout << "16진법 : " << nNum << endl;

	return 0;
}