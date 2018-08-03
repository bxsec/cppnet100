#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock2.h>
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
#endif

#include <stdio.h>
#include <vector>
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	short cmd;       //命令
	short dataLength;  //数据长度
	
};

struct Login:public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult:public DataHeader
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

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		socket = 0;
	}
	int socket;
};

std::vector<SOCKET>g_clients;

int processor(SOCKET sock_client)
{
	char msgRecvMsg[128] = { 0 };
	DataHeader header = {};

	//5. 接收客户端数据
	int nRecvLen = recv(sock_client, (char*)&header, sizeof(header), 0);
	if (nRecvLen <= 0)
	{
		printf("客户端已经退出，任务结束.\n");
#ifdef _WIN32
		closesocket(sock_client);
#else
		close(sock_client);
#endif
		
		return -1;
	}

	//6. 处理请求
	switch (header.cmd)
	{
	case CMD_LOGIN:
	{
		Login login = {};
		recv(sock_client, (char*)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
		printf("收到命令:CMD_LOGIN, 数据长度:%d,userName=%s PassWord=%s\n", login.dataLength, login.userName, login.PassWord);
		//忽略判断用户密码是否正确的过程
		LoginResult ret;
		send(sock_client, (char*)&ret, sizeof(LoginResult), 0);

	}
	break;
	case CMD_LOGOUT:
	{
		Logout logout = {};
		recv(sock_client, (char*)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
		//忽略判断用户密码是否正确的过程
		printf("收到命令:CMD_LOGIN, 数据长度:%d,userName=%s\n", logout.dataLength, logout.userName);

		LogoutResult ret;
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

	return 0;
}

//#pragma comment(lib,"ws2_32.lib")
int main()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(ver, &wsaData) != 0)
	{
		return 0;
	}
#endif

	//1. 建立一个socket
	SOCKET  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//	2. 绑定接收客户端连接的端口 bind
	sockaddr_in addr;
	addr.sin_family = AF_INET;
#ifdef _WIN32
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
#else
	addr.sin_addr.s_addr = ADDR_ANY;
#endif

	addr.sin_port = htons(8000);
	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("server bind error..\n");
#ifdef _WIN32
		WSACleanup();
#endif
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
	

	
	while (true)
	{
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		FD_SET(sock, &fdRead);
		FD_SET(sock, &fdWrite);
		FD_SET(sock, &fdExcept);
		SOCKET maxSock = sock;
		for (size_t n = 0; n < g_clients.size(); ++n)
		{
			FD_SET(g_clients[n], &fdRead);
			FD_SET(g_clients[n], &fdWrite);
			FD_SET(g_clients[n], &fdExcept);

			if (maxSock < g_clients[n])
			{
				maxSock = g_clients[n];
			}

		}

		//nfds是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
		//既是所有文件描述符最大值+1 在windows中这个参数可以写0
		timeval t = { 0,0 };
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
		if (ret < 0)
		{
			printf("select任务结束.\n");
			break;
		}
		if (FD_ISSET(sock, &fdRead))
		{
			FD_CLR(sock, &fdRead);

			sockaddr_in client_sock;
			int nClientLen = sizeof(client_sock);


			SOCKET sock_client = accept(sock, (sockaddr*)&client_sock, &nClientLen);
			if (sock_client != INVALID_SOCKET)
			{
				//inet_ntop()
				printf("有客户端连接,from:%s", inet_ntoa(client_sock.sin_addr));
				printf("有客户端连接\n");

				for (int i = 0; i < g_clients.size(); i++)
				{
					NewUserJoin userJoin;
					userJoin.socket = sock_client;
					send(g_clients[i], (char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(sock_client);
			}
			else
			{
				printf("error socket\n");
			}
		}
		/*for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}*/

		for (size_t n = 0; n < g_clients.size(); n++)
		{
			if(FD_ISSET(g_clients[n],&fdRead))
			{
				if (-1 == processor(g_clients[n]))
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}
		}

	}
	//	6. 关闭socket closesocket
#ifdef _WIN32
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}


	closesocket(sock);

	WSACleanup();
#else
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		close(g_clients[n]);
	}
	close(sock);
#endif

	printf("已退出，结束任务");
	getchar();
	return 0;
}