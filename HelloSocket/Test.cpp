#define WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")

int main()
{
	//����windows�����绷��
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver,&dat);

	//��дͨ�Ŵ���




	//���socket
	WSACleanup();
	return 0;
}