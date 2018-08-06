#include "EasyTcpClient.hpp"

#include <thread>


bool g_bIsRun = true;
//�ͻ�������
const int cCount = 1000;

//�����߳�����
const int tCount = 4;
EasyTcpClient* client[cCount];


void cmdThread()
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

			//client->SendData(&login);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. ���������������
			Logout logout;
			strcpy_s(logout.userName, 32, "lyd");
			//client->SendData(&logout);
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
	}
}

//#pragma comment(lib,"ws2_32.lib")

void sendThread(int id)
{
	//4���߳� ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;


	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}


	for (int n = begin; n < end; n++)
	{
		
		client[n]->Connect("127.0.0.1", 8000);
	}

	Login login;
	strcpy_s(login.userName, "lyd");
	strcpy_s(login.PassWord, "mima");

	while (g_bIsRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->OnRun();
			//printf("�ͻ��˿��д�������ҵ��\n");
			client[n]->SendData(&login);
		}
	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
		delete client[n];
	}
}

int main()
{
	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	

	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();
	}

	

	
	
	
	return 0;
}