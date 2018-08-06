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
#ifdef _WIN32
			WSACleanup();
#endif			
			return ;
		}
		printf("创建套接字成功..\n");
	}

	//连接服务器
	int Connect(const char* ip,unsigned short port)
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
		if (SOCKET_ERROR == connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)))
		{
			printf("<socket=%d>错误，连接服务器<%s:%d>失败\n",sock,ip,port);
#ifdef _WIN32
			WSACleanup();
#endif
			return 0;
		}
		printf("<socket=%d>,连接服务器<%s:%d>成功\n", sock, ip, port);
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
				printf("<socket=%d>select任务结束\n",sock);
				return false;
			}

			if (FD_ISSET(sock, &fdReads))
			{
				FD_CLR(sock, &fdReads);

				if (-1 == RecvData(sock))
				{
					Close();
					printf("<socket=%d>任务结束\n",sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//
#define RECV_BUFF_SIZE 10240
	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}

	//接收数据 需要处理粘包 拆包的问题
	int RecvData(SOCKET sock_client)
	{
		//5. 接收客户端数据
		int nLen = recv(sock_client, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>与服务器断开连接,任务结束。\n", sock_client);
			return -1;
		}
		printf("nLen=%d\n", nLen);
		//将收取的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf+ _lastPos, _szRecv, nLen);
		_lastPos += nLen; //消息缓冲区的数据尾部位置后移
		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		//这时就可以知道当前消息体的长度
		while (_lastPos >= sizeof(DataHeader)) //处理粘包和少包 
		{
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				//剩余未处理消息缓冲区数据的长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				_lastPos = nSize;
			}
			else
			{
				break;
			}
		}

		/*
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
		*/
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
			//LoginResult* ret = (LoginResult*)header;
			//printf("收到服务端信息:CMD_LOGIN_RESULT, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//忽略判断用户密码是否正确的过程
			///LogoutResult* ret = (LogoutResult*)header;
			//printf("收到服务端信息:CMD_LOGOUT_RESULT, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//忽略判断用户密码是否正确的过程
			NewUserJoin* ret = (NewUserJoin*)header;
			printf("收到服务端信息:CMD_NEW_USER_JOIN, 数据长度:%d,SocketId:%d\n", ret->dataLength, ret->socket);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket=%d>收到服务端消息:CMD_ERROR，数据长度:%d\n", sock, header->dataLength);
		}
		break;
		default:
		{
			printf("收到未定义消息，数据长度:%d\n", header->dataLength);
		}
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

