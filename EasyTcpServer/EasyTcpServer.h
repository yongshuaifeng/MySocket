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
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
			printf("<socket = %d>错误，链接套接字<%s:%d>失败...\n", _sock, ip, port);
		}
		else {
			printf("<socket = %d>链接套接字<%s:%d>成功...\n", _sock, ip, port);
		}
		return ret;
	
	}
	//监听端口号
	//接收客户端链接
	//关闭Socket
	//处理网络消息
	//是否工作中
	//接收数据 处理粘包 拆包
	//响应网络消息
	//发送数据

};


#endif // !Easy_Tcp_Server_hpp_

