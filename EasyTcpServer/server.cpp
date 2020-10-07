#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<vector>
#include<algorithm>
#include<thread>

//#pragma comment(lib,"ws2_32.lib")
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};


struct DataHeader 
{
	short datalenth;//datalength 数据长度
	short cmd;//命令

};

//DataPackage
struct Login:public DataHeader
{
	Login() {
		datalenth = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char Password[32];
};

struct LoginResult:public DataHeader
{
	LoginResult() {
		datalenth = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout :public DataHeader
{
	Logout() {
		datalenth = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader
{
	LogoutResult() {
		datalenth = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
		
	}
	int result;
	
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin() {
		datalenth = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		//result = 0;
		sock = 0;
	}
	//int result;
	int sock;
};

std::vector<SOCKET> g_clients;
int processor(SOCKET _cSock) 
{
	//缓冲区
	char szRecv[1024] = {};

	//接收客户端数据
	int nLen = recv(_cSock, (char*)& szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen <= 0) {
		printf("新客户端<Socket=%d>退出，任务结束\n",_cSock);
		return -1;
	}
	//printf("\n收到命令：%d\n 数据长度：%d\n",header.cmd,header.datalenth);
	//if(nLen >= sizeof(DataHeader))//数据量大有用
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		Login* login = (Login*)szRecv;
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		printf("\n收到客户端<Socket=%d>命令：%d\n 数据长度：%d\n UserName = %s\n Password = %s\n",_cSock,login->cmd, login->datalenth, login->userName, login->Password);
		//忽略判断用户名密码的正确性
		LoginResult ret = {};
		send(_cSock, (char*)& header, sizeof(DataHeader), 0);
		send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		Logout* logout = (Logout*)szRecv;
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalenth - sizeof(DataHeader), 0);
		printf("\n收到客户端<Socket=%d>命令：%d\n 数据长度：%d\n UserName = %s\n",_cSock,logout->cmd, logout->datalenth, logout->userName);
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

	return 0;
}


int main()
{
	

	//2 bind绑定用于接收客户端链接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (SOCKET_ERROR == (bind(_sock, (sockaddr*)& _sin, sizeof(_sin))) )
	{
		printf("ERROR,绑定用于接收客户端链接的网络端口失败...\n");
	}
	else {
		printf("绑定端口成功...\n");
	}

	//3 listen监听网络端口
	if (SOCKET_ERROR == (listen(_sock, 3))) {
		printf("ERROR,监听网络端口失败...\n");
	}
	else {
		printf("监听端口成功...\n");
	}


	

	
	while (true) {
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

		
		
		for (int n = 0; n< g_clients.size(); ++n)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		
		//nfds是一个整数值，指fd_set中所有描述符（socket）的范围，而不是数量
		timeval t = {1,0};
		int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (ret == 0) {
			//线程thread
			printf("空闲时间处理其他业务。。。\n");
		}
		if (ret < 0) {
			printf("select任务结束\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			//4 accept等待接收客户端链接
			sockaddr_in clientAddr = {};
			int nAddrlen = sizeof(clientAddr);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrlen);
			
			if (INVALID_SOCKET == _cSock)
			{
				printf("错误，接收无效客户端SOCKET...\n");
			}

			for (int n = 0; n < g_clients.size(); ++n)
			{
				NewUserJoin userJoin;
				send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				//printf("新客户端加入：socket = %d,IP =  %s\n", _cSock, inet_ntoa(clientAddr.sin_addr));
			}
			printf("新客户端加入：socket = %d,IP =  %s\n", _cSock, inet_ntoa(clientAddr.sin_addr));
			g_clients.push_back(_cSock);
		}
		
		for (size_t n = 0; n < fdRead.fd_count; ++n)
		{
			if (processor(fdRead.fd_array[n]) == -1) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
	
	}
	for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
		//FD_SET(g_clients[n], &fdRead);
		closesocket(g_clients[n]);
	}
	//8 关闭套接字closesocket
	closesocket(_sock);
	//----------------------
	//清除Windows socket环境



	//清除socket
	WSACleanup();
	printf("服务器退出，任务结束\n");
	getchar();
	return 0;
}