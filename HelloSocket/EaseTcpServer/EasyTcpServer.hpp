#ifndef _EasyTcpServer_hpp
#define _EasyTcpServer_hpp

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
#endif

#include <stdio.h>
#include <vector>
#include "MsgHeader.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE	10240
#endif

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	
	//消息缓冲区的数据尾部位置
	int _lastPos = 0;
};

class EasyTcpServer
{
private:
	SOCKET sock;
	std::vector<ClientSocket*>m_clients;

public:
	EasyTcpServer()
	{
		sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(ver, &wsaData) != 0)
		{
			return INVALID_SOCKET;
		}
#endif

		if (INVALID_SOCKET != sock)
		{
			printf("<socket=%d>关闭旧连接...\n", sock);
			Close();
		}

		//1. 建立一个socket
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			printf("错误,建立Socket失败...\n");
		}
		else
		{
			printf("建立Socket=<%d>成功...\n", sock);
		}

		return sock;
	}

	//绑定端口号
	int Bind(const char *ip, unsigned short port)
	{
		if (INVALID_SOCKET == sock)
		{
			InitSocket();
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if (ip!="")
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip!="")
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("错误，绑定网络端口<%d>失败...\n", port);
		}
		else
		{
			printf("绑定网络端口<%d>成功...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		//	3. 监听网络端口 listen
		int ret = listen(sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("<Socket=%d>错误,监听失败..\n", sock);
		}
		else
		{
			printf("<Socket=%d>,监听成功..\n", sock);
		}
		return ret;
	}
	//接收客户端连接
	int Accept()
	{
		sockaddr_in client_sock;
		int nClientLen = sizeof(client_sock);


		SOCKET sock_client = accept(sock, (sockaddr*)&client_sock, &nClientLen);
		if (sock_client != INVALID_SOCKET)
		{
			//inet_ntop()
			printf("有客户端连接<socket:%d>,from:%s\n", sock_client, inet_ntoa(client_sock.sin_addr));

			NewUserJoin userJoin;
			userJoin.socket = sock_client;
			SendDataToAll(&userJoin);
			
			m_clients.push_back(new ClientSocket(sock_client));
		}
		else
		{
			printf("error socket\n");
		}

		return sock_client;
	}

	//关闭socket
	void Close()
	{
		//	6. 关闭socket closesocket
#ifdef _WIN32
		for (auto iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			
			closesocket((*iter)->sockfd());
			m_clients.erase(iter);
			delete (*iter);
		}
		closesocket(sock);

		WSACleanup();
#else
		for (auto iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			close((*iter)->sockfd());
			m_clients.erase(iter);
			delete (*iter);
		}
		close(sock);
#endif
		m_clients.clear();
		sock = INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆包

	bool OnRun()
	{
		if (isRun())
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
			for (size_t n = 0; n < m_clients.size(); ++n)
			{
				FD_SET(m_clients[n]->sockfd(), &fdRead);
				FD_SET(m_clients[n]->sockfd(), &fdWrite);
				FD_SET(m_clients[n]->sockfd(), &fdExcept);

				if (maxSock < m_clients[n]->sockfd())
				{
					maxSock = m_clients[n]->sockfd();
				}

			}

			//nfds是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			timeval t = { 0,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
			if (ret < 0)
			{
				printf("select任务结束.\n");
				return false;
			}
			if (FD_ISSET(sock, &fdRead))
			{
				FD_CLR(sock, &fdRead);
				Accept();
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

			for (size_t n = 0; n < m_clients.size(); n++)
			{
				if (FD_ISSET(m_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(m_clients[n]))
					{
						auto iter = m_clients.begin() + n;
						if (iter != m_clients.end())
						{
							delete *iter;
							m_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//缓冲区
	char szRecv[RECV_BUFF_SIZE] = {};

	int RecvData(ClientSocket* sock_client)
	{

		//5. 接收客户端数据
		int nRecvLen = recv(sock_client->sockfd(), (char*)szRecv, RECV_BUFF_SIZE, 0);
		if (nRecvLen <= 0)
		{
			printf("客户端已经退出，任务结束.\n");
			return -1;
		}
		
		memcpy(sock_client->msgBuf() + sock_client->getLastPos(), szRecv, nRecvLen);
		int _lastPos = sock_client->getLastPos() + nRecvLen; //消息缓冲区的数据尾部位置后移
		sock_client->setLastPos(_lastPos);
														 //判断消息缓冲区的数据长度大于消息头DataHeader长度
		//这时就可以知道当前消息体的长度
		while (_lastPos >= sizeof(DataHeader)) //处理粘包和少包 
		{
			char* _szMsgBuf = sock_client->msgBuf();
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				//剩余未处理消息缓冲区数据的长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(sock_client->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				sock_client->setLastPos(nSize);
				_lastPos = nSize;
			}
			else
			{
				break;
			}
		}

		//6. 处理请求
		//OnNetMsg(sock_client->sockfd(), header);

		return 0;
	}

	//响应消息
	virtual void OnNetMsg(SOCKET client, DataHeader* header)
	{

		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			//printf("收到命令:CMD_LOGIN, 数据长度:%d,userName=%s PassWord=%s\n", login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			SendData(client, &ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout *logout = (Logout*)header;

			//忽略判断用户密码是否正确的过程
			//printf("收到命令:CMD_LOGIN, 数据长度:%d,userName=%s\n", logout->dataLength, logout->userName);

			LogoutResult ret;
			SendData(client, &ret);
		}
		break;
		default:
		{
			printf("Server:未解析命令\n");
			DataHeader header = { CMD_ERROR,0 };
			SendData(client, &header);
		}
		break;

		}
	}

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}
	//发送数据
	int SendData(SOCKET client,DataHeader* header)
	{
		int nLen = 0;
		if (isRun() && header)
		{
			nLen = send(client, (char*)header, header->dataLength, 0);
		}
		return nLen;
	}

	void SendDataToAll(DataHeader* header)
	{
		int nLen = 0;
		if (isRun() && header)
		{
			for (auto iter = m_clients.begin(); iter != m_clients.end(); ++iter)
			{
				SendData((*iter)->sockfd(), header);
			}	
		}
	}

};
#endif