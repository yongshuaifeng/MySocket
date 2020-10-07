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
	//构造函数
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}
	
	//初始化socket
	void initSocket()
	{
		//启动Win Sock 2.x环境
#ifdef _WIN32
//启动windows的网络环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//1 建立一个socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket = %d>关闭了旧链接...\n",_sock);
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("建立套接字失败\n");
		}
		else {
			printf("建立套接字成功\n");
		}

	}
	//链接服务器
	int Connect(const char* ip,unsigned short port)
	{
		//2 链接服务器 connect

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
			printf("<socket = %d>错误，链接套接字<%s:%d>失败...\n",_sock,ip,port);
		}
		else {
			printf("<socket = %d>链接套接字<%s:%d>成功...\n",_sock,ip,port);
		}
		return ret;
	}
	//关闭socket
	void Close() 
	{
		if (_sock != INVALID_SOCKET)
		{
			//关闭Win Sock 2.x环境
#ifdef _WIN32
			closesocket(_sock);
			//清除Windows socket环境
			WSACleanup();
#else
			close(_sock);
#endif
			printf("客户端退出，任务结束\n");
			_sock = INVALID_SOCKET;
		}
	}
	//发送数据
	//查询网络消息
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
				//启动线程
				//std::thread t1(cmdThread,_sock);
				/*
				printf("空闲时间处理其他业务。。。\n");
				Login login;
				strcpy(login.userName, "lyd");
				strcpy(login.Password, "lydmm");
				send(_sock, (const char*)& login, sizeof(Login), 0);
				//Sleep(5000);
				*/
			}
			if (ret < 0) {
				printf("<socket = %d>select任务结束1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket = %d>select任务结束2\n", _sock);
					Close();
					return false;
				}

			}
			//printf("空闲时间处理其他业务。。。\n");

			return true;
		
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}


	//接收数据 处理粘包拆包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[1024] = {};

		//接收客户端数据
		int nLen = recv(_cSock, (char*)& szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0) {
			printf("<socket = %d>与服务器断开链接，任务结束\n",_sock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		OnNetMsg(header);

		//printf("\n收到命令：%d\n 数据长度：%d\n",header.cmd,header.datalenth);
		//if(nLen >= sizeof(DataHeader))//数据量大有用
		
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginresult = (LoginResult*)header;
			
			printf("\n<socket = %d>收到服务器消息：CMD_LOGIN_RESULT\n 数据长度：%d\n ", _sock,loginresult->datalenth);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutresult = (LogoutResult*)header;
			
			printf("\n<socket = %d>收到服务器消息：MD_LOGOUT_RESULT\n 数据长度：%d\n ", _sock,logoutresult->datalenth);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userjoin = (NewUserJoin*)header;
			
			printf("\n<socket = %d>收到服务器消息：CMD_NEW_USER_JOIN\n 数据长度：%d\n ",_sock, userjoin->datalenth);
		}
		break;

		}
	
	
	}

	//发送数据
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

