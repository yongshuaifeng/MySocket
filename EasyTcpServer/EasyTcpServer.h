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
#include"MessageHeader.h"
//#include<algorithm>



class EasyTcpServer {
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	
	EasyTcpServer() {}
	virtual ~EasyTcpServer() {}

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
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
			initSocket();
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
		int ret = connect(_sock, (sockaddr*)& _sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("<socket = %d>���������׽���<%s:%d>ʧ��...\n", _sock, ip, port);
		}
		else {
			printf("<socket = %d>�����׽���<%s:%d>�ɹ�...\n", _sock, ip, port);
		}
		return ret;
	
	}
	//�����˿ں�
	//���տͻ�������
	//�ر�Socket
	//����������Ϣ
	//�Ƿ�����
	//�������� ����ճ�� ���
	//��Ӧ������Ϣ
	//��������

};


#endif // !Easy_Tcp_Server_hpp_

