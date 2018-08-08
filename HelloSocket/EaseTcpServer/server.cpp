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
	printf("已退出，结束任务");
	getchar();
	return 0;
}