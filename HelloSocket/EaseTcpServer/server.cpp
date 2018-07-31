#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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


//#pragma comment(lib,"ws2_32.lib")
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(ver, &wsaData) != 0)
	{
		return 0;
	}

	//1. ����һ��socket
	SOCKET  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//	2. �󶨽��տͻ������ӵĶ˿� bind
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
	//	3. ��������˿� listen
	if (SOCKET_ERROR == listen(sock, 5))
	{
		printf("����ʧ��..\n");
	}
	else
	{
		printf("�����ɹ�..\n");
	}
	//	4. �ȴ����ܿͻ������� accept
	sockaddr_in client_sock;
	int nClientLen = sizeof(client_sock);
	

	SOCKET sock_client = accept(sock, (sockaddr*)&client_sock, &nClientLen);
	if (sock_client != INVALID_SOCKET)
	{
		//inet_ntop()
		printf("�пͻ�������,from:%s", inet_ntoa(client_sock.sin_addr));
		printf("�пͻ�������\n");
	}
	else
	{
		printf("error socket\n");
	}

	char msgRecvMsg[128] = { 0 };
	while (true)
	{
		DataHeader header = {};

		//5. ���տͻ�������
		int nRecvLen = recv(sock_client, (char*)&header, sizeof(header), 0);
		if (nRecvLen <= 0)
		{
			printf("�ͻ����Ѿ��˳����������.\n");
			break;
		}
		
		//6. ��������
		switch (header.cmd)
		{
			case CMD_LOGIN:
			{
				Login login = {};
				recv(sock_client, (char*)&login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
				printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s PassWord=%s\n", login.dataLength, login.userName, login.PassWord);
				//�����ж��û������Ƿ���ȷ�Ĺ���
				LoginResult ret;
				send(sock_client, (char*)&ret, sizeof(LoginResult), 0);

			}
				break;
			case CMD_LOGOUT:
			{
				Logout logout = {};
				recv(sock_client, (char*)&logout+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
				//�����ж��û������Ƿ���ȷ�Ĺ���
				printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s\n", logout.dataLength, logout.userName);
				
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
		
	}
	//	6. �ر�socket closesocket
	closesocket(sock_client);
	closesocket(sock);

	WSACleanup();
	printf("���˳�����������");
	getchar();
	return 0;
}