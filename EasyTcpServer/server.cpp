#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include "EasyTcpServer.h"

std::vector<SOCKET> g_clients;



int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr,4567);
	server.Listen(5);

	while (server.isRun()) {
		server.OnRun();
	}
	server.Close();
	printf("服务器退出，任务结束\n");
	getchar();
	return 0;
}