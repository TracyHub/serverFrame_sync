#ifndef PUBLIC_DEFINE_HEAD
#define PUBLIC_DEFINE_HEAD

#include "call_back.h"
#include <dlfcn.h>

static char g_errmsg[128];

#define DLL_GETFUNCADDR(hanlde,output,type,name) do{\
		output = (type)dlsym(handle,name);\
		g_errmsg = dlerror();\
		if (g_errmsg!=NULL){ERROR_LOG("dlsym error,errmsg:%s",g_errmsg);}\
		goto Final;\	
		}while(0)

#define MAXEPOLL 1000 
#define MAX_LISTEN 100
#define MAX_BUFFLEN 1024*100
#define MAX_ARRY_FD 1024
enum NetType{
		TCP_NETTYPE = 1,
		UDP_NETTYPE = 2,
	};

enum ERRCODE{
	//socket error
	ERR_SOCKCREATE = -1000,
	ERR_SOCKBIND,
	ERR_SOCKLISTEN,	
	ERR_SOCKACCEPT,
	ERR_SOCKREAD,
	ERR_SOCKWRITE,
	ERR_TCPSEND,

	//epoll error
	ERR_EPOLLCTL = -2000,
	ERR_EPOLLFULL,
	ERR_EPOLLDEL,

	//msqqueen error
	ERR_MSGCREATE = -3000,
	ERR_MSGSEND，
	ERR_MSGRECV,
	ERR_INITQUEEN,
};

enum{
	epoll_in 	= 0x1,
	epoll_out 	= 0x2,
	epoll_err	= 0x4,
	epoll_hup	= 0x8,
};

enum MsgType{
	SEND_TYPE = 1,//框架回给客户端的消息类型
	RECV_TYPE = 2,//框架收到客户端的消息类型，发送给worker处理
};

typedef FDstruct{
	int remotefd;
	unsigned int clientIp;
	short clientPort;
	unsigned int recvtime;
	char recv[MAX_BUFFLEN];
	char send[MAX_BUFFLEN];
}STfd;

typedef msgstruct{
	unsigned int typeid;//消息类型
	int fd;//与g_arrystfd的坐标对应
	char msgbuff[MAX_BUFFLEN];
	int len;
	}stMsg;

#endif

