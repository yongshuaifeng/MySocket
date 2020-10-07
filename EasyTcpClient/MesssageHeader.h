#ifndef Message_Header_hpp_
#define Message_Header_hpp_


struct DataPackage
{
	int age;
	char name[32];
};


enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};


class DataHeader
{
public:
	short datalenth;//datalength 数据长度
	short cmd;//命令
};

//DataPackage
class Login :public DataHeader
{
public:
	Login() {
		datalenth = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char Password[32];
};

class LoginResult :public DataHeader
{
public:
	LoginResult() {
		datalenth = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

class Logout :public DataHeader
{
public:
	Logout() {
		datalenth = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

class LogoutResult :public DataHeader
{
public:
	LogoutResult() {
		datalenth = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};


class NewUserJoin :public DataHeader
{
public:
	NewUserJoin() {
		datalenth = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		//result = 0;
		sock = 0;
	}
	//int result;
	int sock;
};



#endif // !Message_Header_hpp_

