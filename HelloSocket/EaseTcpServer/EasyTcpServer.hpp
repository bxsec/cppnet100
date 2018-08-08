#ifndef _EasyTcpServer_hpp
#define _EasyTcpServer_hpp

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
	#define FD_SETSIZE			2506
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
#include <thread>
#include <mutex>
#include <atomic>
#include "MsgHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE	10240
#endif

#define _CellServer_THREAD_COUNT	4

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
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	
	//��Ϣ������������β��λ��
	int _lastPos = 0;
};


class INetEvent
{
public:
	//�ͻ����뿪�¼�
	virtual void OnLeave(ClientSocket* pClient) = 0;

};

class CellServer
{
private:
	SOCKET sock;
	std::vector<ClientSocket*>m_clients;
	std::vector<ClientSocket*>m_clientsBuff;
	std::mutex m_Mutex;
	std::thread* m_pThread;
	INetEvent* m_pNetEvent;

public:
	std::atomic_int m_nRecvCount;
public:
	CellServer(SOCKET _sock = INVALID_SOCKET)
	{
		sock = _sock;
		m_pThread = nullptr;
		m_nRecvCount = 0;
		m_pNetEvent = nullptr;
	}
	~CellServer()
	{
		Close();
		sock = INVALID_SOCKET;
	}

	void setNetEventObj(INetEvent* pNetEvent)
	{
		m_pNetEvent = pNetEvent;
	}

	void Close()
	{
		//	6. �ر�socket closesocket
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

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}

	int RecvData(ClientSocket* sock_client)
	{

		//5. ���տͻ�������
		int nRecvLen = recv(sock_client->sockfd(), (char*)szRecv, RECV_BUFF_SIZE, 0);
		if (nRecvLen <= 0)
		{
			printf("�ͻ����Ѿ��˳����������.\n");
			return -1;
		}

		memcpy(sock_client->msgBuf() + sock_client->getLastPos(), szRecv, nRecvLen);
		int _lastPos = sock_client->getLastPos() + nRecvLen; //��Ϣ������������β��λ�ú���
		sock_client->setLastPos(_lastPos);
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
		while (_lastPos >= sizeof(DataHeader)) //����ճ�����ٰ� 
		{
			char* _szMsgBuf = sock_client->msgBuf();
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				//ʣ��δ������Ϣ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(sock_client->sockfd(), header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				sock_client->setLastPos(nSize);
				_lastPos = nSize;
			}
			else
			{
				break;
			}
		}

		//6. ��������
		//OnNetMsg(sock_client->sockfd(), header);

		return 0;
	}

	//������
	char szRecv[RECV_BUFF_SIZE] = {};

	virtual void OnNetMsg(SOCKET client, DataHeader* header)
	{
		/*++_recvCount;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>, clients<%d>,recvCount<%d>\n", t1, sock, m_clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}*/
		m_nRecvCount++;

		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			//printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s PassWord=%s\n", login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			SendData(client, &ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout *logout = (Logout*)header;

			//�����ж��û������Ƿ���ȷ�Ĺ���
			//printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s\n", logout->dataLength, logout->userName);

			LogoutResult ret;
			SendData(client, &ret);
		}
		break;
		default:
		{
			printf("Server:δ��������\n");
			DataHeader header = { CMD_ERROR,0 };
			SendData(client, &header);
		}
		break;

		}
	}

	bool OnRun()
	{
		while (isRun())
		{
			if (m_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex>lock(m_Mutex);
				for (auto pClient : m_clientsBuff)
				{
					m_clients.push_back(pClient);
				}
				m_clientsBuff.clear();
			}

			if (m_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExcept;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			FD_SET(sock, &fdRead);
			FD_SET(sock, &fdWrite);
			FD_SET(sock, &fdExcept);
			SOCKET maxSock = 0;
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

			//nfds��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1 ��windows�������������д0
			timeval t = { 0,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
			if (ret < 0)
			{
				printf("select�������.\n");
				return false;
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
							m_pNetEvent->OnLeave(*iter);
							delete *iter;
							m_clients.erase(iter);

						}
					}
				}
			}
		}
		return false;
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex>lock(m_Mutex);  //�Խ���
		m_clientsBuff.push_back(pClient);
		
	}

	void Start()
	{
		m_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	int SendData(SOCKET client, DataHeader* header)
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

	size_t getClientCount()
	{
		return m_clients.size() + m_clientsBuff.size();
	}

private:

};


class EasyTcpServer : public INetEvent
{
private:
	SOCKET sock;
	std::vector<ClientSocket*>m_clients;
	std::vector<CellServer*> m_cellServers;
	CELLTimestamp _tTime;
	int _recvCount;
public:
	EasyTcpServer()
	{
		sock = INVALID_SOCKET;
		_recvCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//��ʼ��Socket
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
			printf("<socket=%d>�رվ�����...\n", sock);
			Close();
		}

		//1. ����һ��socket
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			printf("����,����Socketʧ��...\n");
		}
		else
		{
			printf("����Socket=<%d>�ɹ�...\n", sock);
		}

		return sock;
	}

	//�󶨶˿ں�
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
			printf("���󣬰�����˿�<%d>ʧ��...\n", port);
		}
		else
		{
			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		//	3. ��������˿� listen
		int ret = listen(sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("<Socket=%d>����,����ʧ��..\n", sock);
		}
		else
		{
			printf("<Socket=%d>,�����ɹ�..\n", sock);
		}
		return ret;
	}
	//���տͻ�������
	int Accept()
	{
		sockaddr_in client_sock;
		int nClientLen = sizeof(client_sock);


		SOCKET sock_client = accept(sock, (sockaddr*)&client_sock, &nClientLen);
		if (sock_client != INVALID_SOCKET)
		{
			//inet_ntop()
			printf("�пͻ�������<socket:%d>,from:%s\n", sock_client, inet_ntoa(client_sock.sin_addr));

			//NewUserJoin userJoin;
			//userJoin.socket = sock_client;
			//SendDataToAll(&userJoin);
			
			//m_clients.push_back(new ClientSocket(sock_client));
			addClientToCellServer(new ClientSocket(sock_client));
		}
		else
		{
			printf("error socket\n");
		}

		return sock_client;
	}

	//���������Start����  ����cellServer��û����
	void addClientToCellServer(ClientSocket* pClient)
	{
		m_clients.push_back(pClient);
		
		auto pMinServer = m_cellServers[0];
		for (auto pCellServer : m_cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);

	}

	void Start()
	{

		for (int n = 0; n < _CellServer_THREAD_COUNT; n++)
		{
			auto ser = new CellServer(sock);
			m_cellServers.push_back(ser);
			ser->Start();
			ser->setNetEventObj(this);
		}
	}

	//�ر�socket
	void Close()
	{
		//	6. �ر�socket closesocket
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
	//�������� ����ճ�� ���

	bool OnRun()
	{
		if (isRun())
		{
			time4msg();

			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExcept;

			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExcept);

			FD_SET(sock, &fdRead);
			//FD_SET(sock, &fdWrite);
			//FD_SET(sock, &fdExcept);
			
			//SOCKET maxSock = sock;
			/*for (size_t n = 0; n < m_clients.size(); ++n)
			{
				FD_SET(m_clients[n]->sockfd(), &fdRead);
				FD_SET(m_clients[n]->sockfd(), &fdWrite);
				FD_SET(m_clients[n]->sockfd(), &fdExcept);

				if (maxSock < m_clients[n]->sockfd())
				{
					maxSock = m_clients[n]->sockfd();
				}

			}*/

			//nfds��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1 ��windows�������������д0
			timeval t = { 0,10 };
			int ret = select(sock + 1, &fdRead, 0/*&fdWrite*/, 0/*&fdExcept*/, &t);
			if (ret < 0)
			{
				printf("select�������.\n");
				return false;
			}
			if (FD_ISSET(sock, &fdRead))
			{
				FD_CLR(sock, &fdRead);
				Accept();
			}
			

			/*
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
			*/
			return true;
		}
		return false;
	}

	//������
	char szRecv[RECV_BUFF_SIZE] = {};

	int RecvData(ClientSocket* sock_client)
	{

		//5. ���տͻ�������
		int nRecvLen = recv(sock_client->sockfd(), (char*)szRecv, RECV_BUFF_SIZE, 0);
		if (nRecvLen <= 0)
		{
			printf("�ͻ����Ѿ��˳����������.\n");
			return -1;
		}
		
		memcpy(sock_client->msgBuf() + sock_client->getLastPos(), szRecv, nRecvLen);
		int _lastPos = sock_client->getLastPos() + nRecvLen; //��Ϣ������������β��λ�ú���
		sock_client->setLastPos(_lastPos);
														 //�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
		while (_lastPos >= sizeof(DataHeader)) //����ճ�����ٰ� 
		{
			char* _szMsgBuf = sock_client->msgBuf();
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				//ʣ��δ������Ϣ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(sock_client->sockfd(), header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				sock_client->setLastPos(nSize);
				_lastPos = nSize;
			}
			else
			{
				break;
			}
		}

		//6. ��������
		//OnNetMsg(sock_client->sockfd(), header);

		return 0;
	}

	//��Ӧ��Ϣ
	virtual void OnNetMsg(SOCKET client, DataHeader* header)
	{
		++_recvCount;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>, clients<%d>,recvCount<%d>\n", t1, sock, m_clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.update();
		}

		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			//printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s PassWord=%s\n", login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			SendData(client, &ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout *logout = (Logout*)header;

			//�����ж��û������Ƿ���ȷ�Ĺ���
			//printf("�յ�����:CMD_LOGIN, ���ݳ���:%d,userName=%s\n", logout->dataLength, logout->userName);

			LogoutResult ret;
			SendData(client, &ret);
		}
		break;
		default:
		{
			printf("Server:δ��������\n");
			DataHeader header = { CMD_ERROR,0 };
			SendData(client, &header);
		}
		break;

		}
	}

	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			int nSockets = 0;
			
			for (auto ser : m_cellServers)
			{
				recvCount += ser->m_nRecvCount;
				nSockets += ser->getClientCount();
				ser->m_nRecvCount = 0;
			}

			printf("time<%lf>,socket<%d>, clients<%d>,recvCount<%d>\n", t1, sock, m_clients.size(), (int)(recvCount/ t1));
			_tTime.update();
		}
	}

	bool isRun()
	{
		return sock != INVALID_SOCKET;
	}
	//��������
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


	virtual void OnLeave(ClientSocket* pClient)
	{
		for (int n = 0; n < m_clients.size(); n++)
		{
			if (m_clients[n] == pClient)
			{
				auto iter = m_clients.begin() + n;
				m_clients.erase(iter);

			}
		}
	}

};
#endif