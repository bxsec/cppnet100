#include <iostream>
#include <stdio.h>
#include <thread>    //C11
#include <string.h>
#include <stdlib.h>
using namespace std;

class cTest
{
public:
	void Test()
	{
		cout << "hello I'm cTest" << endl;
	}
};

void WorkFun()
{
	cout << "Hello,other thread." << endl;
}

int main()
{
	cTest tester;

	thread t(std::mem_fn(&cTest::Test),tester);
	t.join();

	cout << "Hello,main thread." << endl;
	getchar();
	return 0;
}