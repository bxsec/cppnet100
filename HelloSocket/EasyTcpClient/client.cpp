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

//��Ϣͷ
struct DataHeader
{
	short cmd;       //����
	short dataLength;  //���ݳ���
	
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
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(8000);
	if (SOCKET_ERROR == connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("connect ����\n");
		WSACleanup();
		return 0;
	}
	printf("connect �ɹ�..\n");


	while (true)
	{
		//3.������������
		char msgSend[128] = {};
		scanf("%s", msgSend);
		//4.������������
		if (0 == strcmp(msgSend, "exit"))
		{
			break;
		}
		else if(0 == strcmp(msgSend, "login"))
		{
			//5. ���������������
			Login login = { "lyd","mima" };
			DataHeader dh = { CMD_LOGIN, sizeof(login) };
			send(sock, (char*)&dh, sizeof(dh), 0);
			send(sock, (char*)&login, sizeof(login), 0);

			//6.���շ�������������
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. ���������������
			Logout logout = { "lyd" };
			DataHeader dh = { CMD_Logout,sizeof(logout) };
			send(sock, (char*)&dh, sizeof(dh), 0);
			send(sock, (char*)&logout, sizeof(logout), 0);

			//6.���շ�������������
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(sock, (char*)&retHeader, sizeof(DataHeader), 0);
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LogoutResult: %d\n", loginRet.result);
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
		
	}

	printf("�ͻ����ѽ���\n");

	//4. �ر�socket closesocket
	closesocket(sock);
	getchar();

	WSACleanup();

	return 0;
}