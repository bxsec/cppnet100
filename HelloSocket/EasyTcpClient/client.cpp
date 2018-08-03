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

#include <stdio.h>
#include <thread>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//��Ϣͷ
struct DataHeader
{
	short cmd;       //����
	short dataLength;  //���ݳ���

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

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_NEW_USER_JOIN;
		socket = 0;
	}
	int socket;
};

int processor(SOCKET sock_client)
{
	char msgRecvMsg[1024] = { 0 };

	//5. ���տͻ�������
	int nRecvLen = recv(sock_client, (char*)msgRecvMsg, sizeof(DataHeader), 0);
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
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(sock_client, (char*)&msgRecvMsg + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		
		//�����ж��û������Ƿ���ȷ�Ĺ���
		LoginResult* ret = (LoginResult*)msgRecvMsg;
		printf("�յ��������Ϣ:CMD_LOGIN_RESULT, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->result);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(sock_client, (char*)&msgRecvMsg + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		//�����ж��û������Ƿ���ȷ�Ĺ���
		LogoutResult* ret = (LogoutResult*)msgRecvMsg;
		printf("�յ��������Ϣ:CMD_LOGOUT_RESULT, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->result);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(sock_client, (char*)&msgRecvMsg + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		//�����ж��û������Ƿ���ȷ�Ĺ���
		NewUserJoin* ret = (NewUserJoin*)msgRecvMsg;
		printf("�յ��������Ϣ:CMD_NEW_USER_JOIN, ���ݳ���:%d,SocketId:%d\n", ret->dataLength, ret->socket);
	}
	break;
	}

	return 0;
}
bool g_bIsRun = true;
void cmdThread(SOCKET sock)
{
	char msgSend[128] = {};

	while (true)
	{
		scanf("%s", msgSend);

		//4.������������
		if (0 == strcmp(msgSend, "exit"))
		{
			g_bIsRun = false;
			break;
		}
		else if (0 == strcmp(msgSend, "login"))
		{
			//5. ���������������
			Login login;
			login.userName;
			strcpy_s(login.userName, 32, "lyd");
			strcpy_s(login.PassWord, 32, "mima");

			send(sock, (char*)&login, sizeof(login), 0);

			//6.���շ�������������
			LoginResult loginRet;
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. ���������������
			Logout logout;
			strcpy_s(logout.userName, 32, "lyd");
			send(sock, (char*)&logout, sizeof(logout), 0);

			//6.���շ�������������
			LoginResult loginRet = {};
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LogoutResult: %d\n", loginRet.result);
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
	}
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

	//1. ����socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock)
	{
		printf("����sockʧ��\n");
		WSACleanup();
		return 0;
	}
	printf("�����׽��ֳɹ�..\n");

	//2. ���ӷ����� connect
	sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
#ifdef _WIN32
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
	serverAddr.sin_addr.s_addr = inet_addr("192.168.229.1");
#endif
	
	serverAddr.sin_port = htons(8000);
	if (SOCKET_ERROR == connect(sock+1, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("connect ����\n");
#ifdef _WIN32
		WSACleanup();
#endif
		return 0;
	}
	printf("connect �ɹ�..\n");

	std::thread t1(cmdThread, sock);
	t1.detach();
	while (g_bIsRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);

		FD_SET(sock, &fdReads);
		timeval tv = { 1,0 };
		int ret = select(sock + 1, &fdReads, 0, 0, &tv);
		if (ret < 0)
		{
			DWORD dwError = GetLastError();
			printf("select�������\n");
			break;
		}

		if (FD_ISSET(sock, &fdReads))
		{
			FD_CLR(sock, &fdReads);

			if (-1 == processor(sock))
			{
				break;
			}
		}

		
		
		

		//printf("�ͻ��˿��д�������ҵ��\n");
		
	}

	printf("�ͻ����ѽ���\n");

	//4. �ر�socket closesocket
	
	getchar();
#ifdef _WIN32
	closesocket(sock);
	WSACleanup();
#else
	close(sock);
#endif
	return 0;
}