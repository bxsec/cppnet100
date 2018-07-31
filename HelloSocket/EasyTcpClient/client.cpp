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

	printf("�ͻ����ѽ���\n");

	//4. �ر�socket closesocket
	closesocket(sock);
	getchar();

	WSACleanup();

	return 0;
}