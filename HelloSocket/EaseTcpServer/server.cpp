#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

struct DataPackage
{
	int age;
	char name[32];
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
	while(true)
	{
		//5. ���տͻ�������
		int nRecvLen = recv(sock_client, msgRecvMsg, 128, 0);
		if (nRecvLen <= 0)
		{
			printf("�ͻ����Ѿ��˳����������.\n");
			break;
		}
		//6. ��������
		if (0 == strcmp(msgRecvMsg, "getInfo") )
		{
			DataPackage dp = {80,"������"};

			send(sock_client, (const char*)&dp, sizeof(dp), 0);
		}
		else
		{
			//	7. ��ͻ��˷���һ������send
			char msgBuf[] = "???";
			send(sock_client, msgBuf, strlen(msgBuf) + 1, 0);
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