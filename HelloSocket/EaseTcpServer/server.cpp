#include "EasyTcpServer.hpp"


//#pragma comment(lib,"ws2_32.lib")
int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind("", 8000);
	server.Listen(5);
	server.Start();
	while (server.isRun())
	{
		server.OnRun();

	}
	server.Close();
	printf("���˳�����������");
	getchar();
	return 0;
}