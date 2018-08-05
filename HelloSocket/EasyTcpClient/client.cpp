#include "EasyTcpClient.hpp"

#include <thread>


bool g_bIsRun = true;
void cmdThread(EasyTcpClient* client)
{
	char msgSend[128] = {};

	while (true)
	{
		scanf("%s", msgSend);

		//4.处理请求命令
		if (0 == strcmp(msgSend, "exit"))
		{
			client->Close();
			break;
		}
		else if (0 == strcmp(msgSend, "login"))
		{
			//5. 向服务器发送请求
			Login login;
			login.userName;
			strcpy_s(login.userName, 32, "lyd");
			strcpy_s(login.PassWord, 32, "mima");

			client->SendData(&login);

		}
		else if (0 == strcmp(msgSend, "logout"))
		{
			//5. 向服务器发送请求
			Logout logout;
			strcpy_s(logout.userName, 32, "lyd");
			client->SendData(&logout);
		}
		else
		{
			printf("不支持的命令\n");
		}
	}
}

//#pragma comment(lib,"ws2_32.lib")
int main()
{
	EasyTcpClient client;
	client.initSocket();
	client.Connect("127.0.0.1", 8000);
	/*std::thread t1(cmdThread, &client);
	t1.detach();*/
	Login login;
	strcpy_s(login.userName, "lyd");
	strcpy_s(login.PassWord, "mima");

	while (client.isRun())
	{
		client.OnRun();
		//printf("客户端空闲处理其他业务\n");
		client.SendData(&login);
	}
	client.Close();

	
	return 0;
}