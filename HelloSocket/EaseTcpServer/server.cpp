#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

	//1. 建立一个socket
	SOCKET  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//	2. 绑定接收客户端连接的端口 bind
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	addr.sin_port = htons(8000);
	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("server bind error..\n");
		WSACleanup();
		return 0;
	}
	else
	{
		printf("bind successful.\n");
	}
	//	3. 监听网络端口 listen
	if (SOCKET_ERROR == listen(sock, 5))
	{
		printf("监听失败..\n");
	}
	else
	{
		printf("监听成功..\n");
	}
	//	4. 等待接受客户端连接 accept
	sockaddr_in client_sock;
	int nClientLen = sizeof(client_sock);
	

	SOCKET sock_client = accept(sock, (sockaddr*)&client_sock, &nClientLen);
	if (sock_client != INVALID_SOCKET)
	{
		//inet_ntop()
		printf("有客户端连接,from:%s", inet_ntoa(client_sock.sin_addr));
		printf("有客户端连接\n");
	}
	else
	{
		printf("error socket\n");
	}

	char msgRecvMsg[128] = { 0 };
	while (true)
	{
		DataHeader header = {};

		//5. 接收客户端数据
		int nRecvLen = recv(sock_client, (char*)&header, sizeof(header), 0);
		if (nRecvLen <= 0)
		{
			printf("客户端已经退出，任务结束.\n");
			break;
		}
		printf("收到命令:%d, 数据长度:%d\n", header.cmd, header.dataLength);
		//6. 处理请求
		switch (header.cmd)
		{
			case CMD_LOGIN:
			{
				Login login = {};
				recv(sock_client, (char*)&login, sizeof(Login), 0);
				//忽略判断用户密码是否正确的过程
				LoginResult ret = {1};
				send(sock_client, (char*)&header, sizeof(DataHeader), 0);
				send(sock_client, (char*)&ret, sizeof(LoginResult), 0);

			}
				break;
			case CMD_Logout:
			{
				Logout logout = {};
				recv(sock_client, (char*)&logout, sizeof(Logout), 0);
				//忽略判断用户密码是否正确的过程
				LogoutResult ret = { 1 };
				send(sock_client, (char*)&header, sizeof(DataHeader), 0);
				send(sock_client, (char*)&ret, sizeof(LoginResult), 0);
			}
				break;
			default:
			{
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(sock_client, (char*)&header, sizeof(DataHeader), 0);
			}
			break;
		}
		
	}
	//	6. 关闭socket closesocket
	closesocket(sock_client);
	closesocket(sock);

	WSACleanup();
	printf("已退出，结束任务");
	getchar();
	return 0;
}