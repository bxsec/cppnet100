#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

struct Login
{
	char userName[32];
	char PassWord[32];
};

struct LoginResult
{
	int result;
};

struct Logout
{
	char userName[32];
};

struct LogoutResult
{
	int result;
};

enum CMD
{
	CMD_LOGIN,
	CMD_Logout,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	short cmd;       //命令
	short dataLength;  //数据长度
	
};

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
		else if(0 == strcmp(msgSend, "login"))
		{
			//5. 向服务器发送请求
			Login login = { "lyd","mima" };
			DataHeader dh = { CMD_LOGIN, sizeof(login) };
			send(sock, (char*)&dh, sizeof(dh), 0);
			send(sock, (char*)&login, sizeof(login), 0);

			//6.接收服务器返回数据
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. 向服务器发送请求
			Logout logout = { "lyd" };
			DataHeader dh = { CMD_Logout,sizeof(logout) };
			send(sock, (char*)&dh, sizeof(dh), 0);
			send(sock, (char*)&logout, sizeof(logout), 0);

			//6.接收服务器返回数据
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LogoutResult: %d\n", loginRet.result);
		}
		else
		{
			printf("不支持的命令\n");
		}
		
	}

	printf("客户端已结束\n");

	//4. 关闭socket closesocket
	closesocket(sock);
	getchar();

	WSACleanup();

	return 0;
}