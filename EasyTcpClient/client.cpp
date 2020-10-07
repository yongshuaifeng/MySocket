#define WIN32_LEAN_AND_MEAN
#include"EasyTcpClient.h"
#include<thread>

bool g_bRun = true;

void cmdThread(EasyTcpClient* client)
{
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出线程\n");
			return;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "lyd");
			strcpy(login.Password, "lydmm");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyd");
			client->SendData(&logout);

		}
		else {
			printf("不支持的命令\n");
		}
	}

	return;
}



int main()
{
	EasyTcpClient client;
	//client.initSocket();
	client.Connect("127.0.0.1", 4567);

	//启动UI线程
	std::thread t1(cmdThread, &client);
	t1.detach();

	
	EasyTcpClient client2;
	client2.Connect("127.0.0.1", 4567);
	std::thread t2(cmdThread, &client2);
	t2.detach();
	
	
	

	//3 接收服务器信息 recv
	char recvBuf[256] = {};
	char cmdBuf[128] = {};

	while (client.isRun() || client2.isRun()) {
		client.OnRun();
		client2.OnRun();
	}

	client.Close();
	client2.Close();

	getchar();
	return 0;
}
