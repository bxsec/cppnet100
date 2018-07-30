#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

//#pragma comment(lib,"ws2_32.lib")
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(ver, &wsaData) != 0)
	{
		return 0;
	}

	//1. 建立socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock)
	{
		printf("创建sock失败\n");
		WSACleanup();
		return 0;
	}
	printf("创建套接字成功..\n");

	//2. 连接服务器 connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(8000);
	if (SOCKET_ERROR == connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("connect 错误\n");
		WSACleanup();
		return 0;
	}
	printf("connect 成功..\n");


	while (true)
	{
		//3.输入请求命令
		char msgSend[128] = {};
		scanf("%s", msgSend);
		//4.处理请求命令
		if (0 == strcmp(msgSend, "exit"))
		{
			break;
		}
		else
		{
			//5. 向服务器发送请求
			send(sock, msgSend, strlen(msgSend) + 1, 0);
		}
		
		//6. 接收 recv
		char buf[128] = { 0 };
		int ret = recv(sock, buf, 1024, 0);
		if (ret > 0)
		{
			buf[ret] = '\0';
			printf("server:%s\n", buf);
		}
	}

	printf("客户端已结束\n");

	//4. 关闭socket closesocket
	closesocket(sock);
	getchar();

	WSACleanup();

	return 0;
}