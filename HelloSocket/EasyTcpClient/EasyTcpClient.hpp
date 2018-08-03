#ifndef _EasyTcpClient_hpp
#define _EasyTcpClient_hpp

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock2.h>
	
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR	(-1)
#endif

#include<stdio.h>
#include "MsgHeader.hpp"
class EasyTcpClient
{
	SOCKET sock;
public:
	EasyTcpClient()
	{
		sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient()
	{
		Close();
	}

	//初始化socket
	void initSocket()
	{
		//启动win socket 2.x环境
		
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(ver, &wsaData) != 0)
		{
			return;
		}
#endif
		if (sock != INVALID_SOCKET)
		{
			printf("socket=%d>关闭旧连接...\n", sock);
			Close();
		}

		//1. 建立socket
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			printf("创建sock失败\n");
			WSACleanup();
			return ;
		}
		printf("创建套接字成功..\n");
	}

	//连接服务器
	int Connect(char* ip,unsigned short port)
	{
		if (sock == INVALID_SOCKET)
		{
			initSocket();
		}


		//2. 连接服务器 connect
		sockaddr_in serverAddr;

		serverAddr.sin_family = AF_INET;
#ifdef _WIN32
		serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		serverAddr.sin_addr.s_addr = inet_addr(ip);
#endif

		serverAddr.sin_port = htons(port);
		if (SOCKET_ERROR == connect(sock + 1, (sockaddr*)&serverAddr, sizeof(serverAddr)))
		{
			printf("connect 错误\n");
#ifdef _WIN32
			WSACleanup();
#endif
			return 0;
		}
		printf("connect 成功..\n");
		return 0;
	}

	//关闭socket
	void Close()
	{
		//清理Win Sock 2.x环境
		if(sock != INVALID_SOCKET)
		{
#ifdef _WIN32
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		sock = INVALID_SOCKET;
		}
	}
	//处理网络数据
	bool OnRun()
	{
		if(isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);

			FD_SET(sock, &fdReads);
			timeval tv = { 1,0 };
			int ret = select(sock + 1, &fdReads, 0, 0, &tv);
			if (ret < 0)
			{

				printf("<socket=%d>任务结束\n",sock);
				return false;
			}

			if (FD_ISSET(sock, &fdReads))
			{
				FD_CLR(sock, &fdReads);

				if (-1 == RecvData(sock))
				{
					printf("<socket=%d>任务结束\n",sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}

	//接收数据 需要处理粘包 拆包的问题
	int RecvData(SOCKET sock_client)
	{
		char msgRecvMsg[1024] = { 0 };

		//5. 接收客户端数据
		int nRecvLen = recv(sock_client, (char*)msgRecvMsg, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)msgRecvMsg;
		if (nRecvLen <= 0)
		{
			printf("服务器已经退出，任务结束.\n");
#ifdef _WIN32
			closesocket(sock_client);
#else
			close(sock_client);
#endif
			return -1;
		}

		//6. 处理请求
		recv(sock_client, (char*)&msgRecvMsg + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);

		return 0;
	}

	//响应网络数据
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			

			//忽略判断用户密码是否正确的过程
			LoginResult* ret = (LoginResult*)header;
			printf("收到服务端信息:CMD_LOGIN_RESULT, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//忽略判断用户密码是否正确的过程
			LogoutResult* ret = (LogoutResult*)header;
			printf("收到服务端信息:CMD_LOGOUT_RESULT, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//忽略判断用户密码是否正确的过程
			NewUserJoin* ret = (NewUserJoin*)header;
			printf("收到服务端信息:CMD_NEW_USER_JOIN, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->socket);
		}
		break;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		int nLen = 0;
		if (isRun() && header)
		{
			nLen = send(sock, (char*)header, header->dataLength, 0);
		}
		return nLen;
	}
private:

};






#endif

