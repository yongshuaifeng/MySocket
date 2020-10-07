#ifndef Easy_Tcp_Server_hpp_
#define Easy_Tcp_Server_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS 
	#include<Windows.h>
	#include<WinSock2.h>
	#pragma comment (lib,"ws2_32.lib")
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR           (-1)
#endif // !_WIN32

#include<thread>
#include<stdio.h>
#include<vector>
#include"../EasyTcpClient/MesssageHeader.h"

//#include<algorithm>



class EasyTcpServer {
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	
	EasyTcpServer() { _sock = INVALID_SOCKET; }
	virtual ~EasyTcpServer() {
		Close();
	}

	//��ʼ��Socket
	void InitSocket()
	{
#ifdef _WIN32
		//����windows�����绷��
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

#endif // _WIN32
		// ��Socket API��������TCP�����
		//1 ����һ��socket�׽���
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", _sock);
			//Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Sockʧ��...\n");
		}
		else {
			printf("����Socket=<%d>�ɹ�...\n",_sock);
		}
	}
	//��IP�Ͷ˿ں�
	int Bind(const char* ip,unsigned short port) 
	{
		//2 ���ӷ����� connect
		
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if(ip)
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		else
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		if (ip)
			_sin.sin_addr.s_addr = inet_addr(ip);
		else
			_sin.sin_addr.s_addr = INADDR_ANY;
#endif
		int ret = bind(_sock,(sockaddr*)& _sin,sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("<socket = %d>���󣬰��׽���<%s:%d>ʧ��...\n", _sock, ip, port);
		}
		else {
			printf("<socket = %d>���׽���<%s:%d>�ɹ�...\n", _sock, ip, port);
		}
		return ret;
	
	}
	//�����˿ں�
	int Listen(int n) {
		//3 listen��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket = < %d >ERROR,��������˿�ʧ��...\n",_sock);
		}
		else {
			printf("Socket = < %d >�����˿ڳɹ�...\n",_sock);
		}
		return ret;
	}
	//���տͻ�������
	SOCKET Accept() {
		//4 accept�ȴ����տͻ�������
		sockaddr_in clientAddr = {};
		int nAddrlen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrlen);
#elif
		_cSock = accept(_sock, (sockaddr*)& clientAddr, (socklen_t*)&nAddrlen);
#endif
		if (INVALID_SOCKET == _cSock)
		{
			printf("socket=<%d>���󣬽�����Ч�ͻ���SOCKET...\n",_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			printf("socket=<%d>�¿ͻ��˼��룺socket = %d,IP =  %s\n", _sock, _cSock, inet_ntoa(clientAddr.sin_addr));
			g_clients.push_back(_cSock);
		}
		return _cSock;
	}
	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				closesocket(g_clients[n]);
			}
			//�ر�Win Sock 2.x����
#ifdef _WIN32
			//8 �ر��׽���closesocket
			closesocket(_sock);
			//----------------------
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}
	//����������Ϣ
	bool OnRun() {
		if (isRun()) {
			//������socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);



			for (unsigned n = 0; n < g_clients.size(); ++n)
			{
				FD_SET(g_clients[n], &fdRead);
			}

			//nfds��һ������ֵ��ָfd_set��������������socket���ķ�Χ������������
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret == 0) {
				//�߳�thread
				printf("����ʱ�䴦������ҵ�񡣡���\n");
			}
			if (ret < 0) {
				printf("select�������\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				Accept();
			}

			for (size_t n = 0; n < fdRead.fd_count; ++n)
			{
				if (RecvData(fdRead.fd_array[n]) == -1) {
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}


		
		
		}
		return false;
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�������� ����ճ�� ���
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[1024] = {};

		//���տͻ�������
		int nLen = recv(_cSock, (char*)& szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0) {
			printf("�¿ͻ���<Socket=%d>�˳����������\n", _cSock);
			return -1;
		}
		//printf("\n�յ����%d\n ���ݳ��ȣ�%d\n",header.cmd,header.datalenth);
		//if(nLen >= sizeof(DataHeader))//������������
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);

		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			
			printf("\n�յ��ͻ���<Socket=%d>���%d\n ���ݳ��ȣ�%d\n UserName = %s\n Password = %s\n", _cSock, login->cmd, login->datalenth, login->userName, login->Password);
			//�����ж��û����������ȷ��
			LoginResult ret = {};
			send(_cSock, (char*)& header, sizeof(DataHeader), 0);
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			
			printf("\n�յ��ͻ���<Socket=%d>���%d\n ���ݳ��ȣ�%d\n UserName = %s\n", _cSock, logout->cmd, logout->datalenth, logout->userName);
			//�����ж��û����������ȷ��
			LogoutResult ret = {};
			//send(_cSock, (char*)& header, sizeof(DataHeader), 0);
			send(_cSock, (char*)& ret, sizeof(LogoutResult), 0);
		}
		break;
		default:
		{
			header->cmd = CMD_ERROR;
			header->datalenth = 0;
			send(_cSock, (char*)& header, sizeof(header), 0);
		}
		break;
		}

	
	}
	//����ָ������
	int SendData(SOCKET _cSock,DataHeader* header) {
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->datalenth, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ��
	void SendDataToAll(DataHeader* header) {
		for (unsigned n = 0; n < g_clients.size(); ++n)
		{
			SendData(g_clients[n], header);
		}
	}

};


#endif // !Easy_Tcp_Server_hpp_

