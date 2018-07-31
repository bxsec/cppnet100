#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	short cmd;       //命令
	short dataLength;  //数据长度

};

struct Login :public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
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
			Login login;
			login.userName;
			strcpy_s(login.userName, 32, "lyd");
			strcpy_s(login.PassWord, 32, "mima");

			send(sock, (char*)&login, sizeof(login), 0);

			//6.接收服务器返回数据

			LoginResult loginRet;
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. 向服务器发送请求
			Logout logout;
			strcpy_s(logout.userName, 32, "lyd");
			send(sock, (char*)&logout, sizeof(logout), 0);

			//6.接收服务器返回数据
			LoginResult loginRet = {};
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