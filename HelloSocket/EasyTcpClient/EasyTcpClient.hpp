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

	//��ʼ��socket
	void initSocket()
	{
		//����win socket 2.x����
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
			printf("socket=%d>�رվ�����...\n", sock);
			Close();
		}

		//1. ����socket
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			printf("����sockʧ��\n");
#ifdef _WIN32
			WSACleanup();
#endif			
			return ;
		}
		printf("�����׽��ֳɹ�..\n");
	}

	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		if (sock == INVALID_SOCKET)
		{
			initSocket();
		}

		//2. ���ӷ����� connect
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
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ��\n",sock,ip,port);
#ifdef _WIN32
			WSACleanup();
#endif
			return 0;
		}
		printf("<socket=%d>,���ӷ�����<%s:%d>�ɹ�\n", sock, ip, port);
		return 0;
	}

	//�ر�socket
	void Close()
	{
		//����Win Sock 2.x����
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
	//������������
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
				printf("<socket=%d>select�������\n",sock);
				return false;
			}

			if (FD_ISSET(sock, &fdReads))
			{
				FD_CLR(sock, &fdReads);

				if (-1 == RecvData(sock))
				{
					Close();
					printf("<socket=%d>�������\n",sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//
#define RECV_BUFF_SIZE 10240
	//���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}

	//�������� ��Ҫ����ճ�� ���������
	int RecvData(SOCKET sock_client)
	{
		//5. ���տͻ�������
		int nLen = recv(sock_client, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>��������Ͽ�����,���������\n", sock_client);
			return -1;
		}
		printf("nLen=%d\n", nLen);
		//����ȡ�����ݿ�������Ϣ������
		memcpy(_szMsgBuf+ _lastPos, _szRecv, nLen);
		_lastPos += nLen; //��Ϣ������������β��λ�ú���
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
		while (_lastPos >= sizeof(DataHeader)) //����ճ�����ٰ� 
		{
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				//ʣ��δ������Ϣ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������ʣ��δ��������ǰ��
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
			printf("�������Ѿ��˳����������.\n");
#ifdef _WIN32
			closesocket(sock_client);
#else
			close(sock_client);
#endif
			return -1;
		}

		//6. ��������
		recv(sock_client, (char*)&msgRecvMsg + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		*/
		return 0;
	}

	//��Ӧ��������
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			

			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LoginResult* ret = (LoginResult*)header;
			//printf("�յ��������Ϣ:CMD_LOGIN_RESULT, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//�����ж��û������Ƿ���ȷ�Ĺ���
			///LogoutResult* ret = (LogoutResult*)header;
			//printf("�յ��������Ϣ:CMD_LOGOUT_RESULT, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->result);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//�����ж��û������Ƿ���ȷ�Ĺ���
			NewUserJoin* ret = (NewUserJoin*)header;
			printf("�յ��������Ϣ:CMD_NEW_USER_JOIN, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->socket);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket=%d>�յ��������Ϣ:CMD_ERROR�����ݳ���:%d\n", sock, header->dataLength);
		}
		break;
		default:
		{
			printf("�յ�δ������Ϣ�����ݳ���:%d\n", header->dataLength);
		}
		}
	}

	//��������
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

