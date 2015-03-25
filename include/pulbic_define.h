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

enum NetType{
		TCP_NETTYPE = 1,
		UDP_NETTYPE = 2,
	};

enum ERRCODE{
	//socket error
	ERR_SOCKCREATE = -1000;
	ERR_SOCKBIND,
	ERR_SOCKLISTEN,	
	ERR_SOCKACCEPT,
	ERR_SOCKREAD,
	ERR_SOCKWRITE,

	//epoll error
	ERR_EPOLLCTL = -2000;
	ERR_EPOLLFULL,
	ERR_EPOLLDEL,
};

enum{
	epoll_in 	= 0x1,
	epoll_out 	= 0x2,
	epoll_err	= 0x4,
	epoll_hup	= 0x8,
};

#endif

