#define WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")

int main()
{
	//启动windows的网络环境
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver,&dat);

	//编写通信代码




	//清除socket
	WSACleanup();
	return 0;
}