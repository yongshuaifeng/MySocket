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

	//初始化Socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动windows的网络环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

#endif // _WIN32
		// 用Socket API建立简易TCP服务端
		//1 建立一个socket套接字
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭就链接...\n", _sock);
			//Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Sock失败...\n");
		}
		else {
			printf("建立Socket=<%d>成功...\n",_sock);
		}
	}
	//绑定IP和端口号
	int Bind(const char* ip,unsigned short port) 
	{
		//2 链接服务器 connect
		
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
			printf("<socket = %d>错误，绑定套接字<%s:%d>失败...\n", _sock, ip, port);
		}
		else {
			printf("<socket = %d>绑定套接字<%s:%d>成功...\n", _sock, ip, port);
		}
		return ret;
	
	}
	//监听端口号
	int Listen(int n) {
		//3 listen监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket = < %d >ERROR,监听网络端口失败...\n",_sock);
		}
		else {
			printf("Socket = < %d >监听端口成功...\n",_sock);
		}
		return ret;
	}
	//接收客户端链接
	SOCKET Accept() {
		//4 accept等待接收客户端链接
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
			printf("socket=<%d>错误，接收无效客户端SOCKET...\n",_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			printf("socket=<%d>新客户端加入：socket = %d,IP =  %s\n", _sock, _cSock, inet_ntoa(clientAddr.sin_addr));
			g_clients.push_back(_cSock);
		}
		return _cSock;
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				closesocket(g_clients[n]);
			}
			//关闭Win Sock 2.x环境
#ifdef _WIN32
			//8 关闭套接字closesocket
			closesocket(_sock);
			//----------------------
			//清除Windows socket环境
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}
	//处理网络消息
	bool OnRun() {
		if (isRun()) {
			//伯克利socket
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

			//nfds是一个整数值，指fd_set中所有描述符（socket）的范围，而不是数量
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret == 0) {
				//线程thread
				printf("空闲时间处理其他业务。。。\n");
			}
			if (ret < 0) {
				printf("select任务结束\n");
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
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[1024] = {};

		//接收客户端数据
		int nLen = recv(_cSock, (char*)& szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0) {
			printf("新客户端<Socket=%d>退出，任务结束\n", _cSock);
			return -1;
		}
		//printf("\n收到命令：%d\n 数据长度：%d\n",header.cmd,header.datalenth);
		//if(nLen >= sizeof(DataHeader))//数据量大有用
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);

		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			
			printf("\n收到客户端<Socket=%d>命令：%d\n 数据长度：%d\n UserName = %s\n Password = %s\n", _cSock, login->cmd, login->datalenth, login->userName, login->Password);
			//忽略判断用户名密码的正确性
			LoginResult ret = {};
			send(_cSock, (char*)& header, sizeof(DataHeader), 0);
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			
			printf("\n收到客户端<Socket=%d>命令：%d\n 数据长度：%d\n UserName = %s\n", _cSock, logout->cmd, logout->datalenth, logout->userName);
			//忽略判断用户名密码的正确性
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
	//发送指定数据
	int SendData(SOCKET _cSock,DataHeader* header) {
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->datalenth, 0);
		}
		return SOCKET_ERROR;
	}
	//群发
	void SendDataToAll(DataHeader* header) {
		for (unsigned n = 0; n < g_clients.size(); ++n)
		{
			SendData(g_clients[n], header);
		}
	}

};


#endif // !Easy_Tcp_Server_hpp_

