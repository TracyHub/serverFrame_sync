#ifndef NET_HEAD
#define NET_HEAD

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include "public_define.h"
#include "queenmsg.h"

extern STfd g_arrystfd[MAX_ARRY_FD];

namespace NetBaseSpace{

//define event struct
struct stEvent{
	int 			fd;//sockfd or pipefd
	callback* 		pcallback;
	int 			event;
	unsigned int 	lastactivetime;
	int 			status;//1:in epoll wait list,0:not in epoll wait list
	int 			offset;
	int 			len;
	int 			queenid;
	char			buff[1024*10];			
};

class NetBase{
	public:
			NetBase(int iport,int nettype);
			~NetBase(){}
			int Init();
			int AddEvents(struct stEvent* pev);
			int DelEvents(struct stEvent* pev);
			void SetEvents(int fd, int events, callback* pFuncCallback,int status, stEvent* event);
			int RecvData(int fd, int events, void* argv);
			int SendData(int fd, int events, void* argv);
			int AcceptConn(int fd, int events, void* argv);
			int WaitForEvents(int timeout);
			int NetLoop(int eventCount);
			int Tcp_Send_N(int sockfd, void *buf, int total);
	protected:
			int m_efd;
			int m_sockfd;
			struct epoll_event* m_pevent;
			struct epoll_event m_ev;
			char m_sendbuff[1024*100];
			char m_recvbuff[1024*100];
			//struct stEvent m_stevent[MAXEPOLL];
			int m_iport;
			int m_Recvpipe;
			NetType m_eNettype;
};
};

#endif