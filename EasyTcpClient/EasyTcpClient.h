#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include<Windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include"MesssageHeader.h"

class EasyTcpClient
{
public:
	SOCKET _sock;
public:
	//���캯��
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	//����������
	virtual ~EasyTcpClient()
	{
		Close();
	}
	
	//��ʼ��socket
	void initSocket()
	{
		//����Win Sock 2.x����
#ifdef _WIN32
//����windows�����绷��
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//1 ����һ��socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket = %d>�ر��˾�����...\n",_sock);
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("�����׽���ʧ��\n");
		}
		else {
			printf("�����׽��ֳɹ�\n");
		}

	}
	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		//2 ���ӷ����� connect

		if (INVALID_SOCKET == _sock)
		{
			initSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)& _sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("<socket = %d>���������׽���<%s:%d>ʧ��...\n",_sock,ip,port);
		}
		else {
			printf("<socket = %d>�����׽���<%s:%d>�ɹ�...\n",_sock,ip,port);
		}
		return ret;
	}
	//�ر�socket
	void Close() 
	{
		if (_sock != INVALID_SOCKET)
		{
			//�ر�Win Sock 2.x����
#ifdef _WIN32
			closesocket(_sock);
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
			printf("�ͻ����˳����������\n");
			_sock = INVALID_SOCKET;
		}
	}
	//��������
	//��ѯ������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret == 0) {
				//�����߳�
				//std::thread t1(cmdThread,_sock);
				/*
				printf("����ʱ�䴦������ҵ�񡣡���\n");
				Login login;
				strcpy(login.userName, "lyd");
				strcpy(login.Password, "lydmm");
				send(_sock, (const char*)& login, sizeof(Login), 0);
				//Sleep(5000);
				*/
			}
			if (ret < 0) {
				printf("<socket = %d>select�������1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket = %d>select�������2\n", _sock);
					Close();
					return false;
				}

			}
			//printf("����ʱ�䴦������ҵ�񡣡���\n");

			return true;
		
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}


	//�������� ����ճ�����
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[1024] = {};

		//���տͻ�������
		int nLen = recv(_cSock, (char*)& szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0) {
			printf("<socket = %d>��������Ͽ����ӣ��������\n",_sock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		OnNetMsg(header);

		//printf("\n�յ����%d\n ���ݳ��ȣ�%d\n",header.cmd,header.datalenth);
		//if(nLen >= sizeof(DataHeader))//������������
		
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginresult = (LoginResult*)header;
			
			printf("\n<socket = %d>�յ���������Ϣ��CMD_LOGIN_RESULT\n ���ݳ��ȣ�%d\n ", _sock,loginresult->datalenth);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutresult = (LogoutResult*)header;
			
			printf("\n<socket = %d>�յ���������Ϣ��MD_LOGOUT_RESULT\n ���ݳ��ȣ�%d\n ", _sock,logoutresult->datalenth);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userjoin = (NewUserJoin*)header;
			
			printf("\n<socket = %d>�յ���������Ϣ��CMD_NEW_USER_JOIN\n ���ݳ��ȣ�%d\n ",_sock, userjoin->datalenth);
		}
		break;

		}
	
	
	}

	//��������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			if (CMD_LOGIN == header->cmd) {
				Login* login = (Login*)header;
				return send(_sock, (const char*)login, login->datalenth, 0);
			}
			else if (CMD_LOGOUT == header->cmd) {
				Login* logout = (Login*)header;
				return send(_sock, (const char*)logout, logout->datalenth, 0);
			}
			
		}
		return SOCKET_ERROR;
	}

private:

};


#endif // !_EasyTcpClient_hpp_

