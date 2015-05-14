#include "netbase.h"

using namespace NetBaseSpace;

NetBase::NetBase(int iport,int nettype){
	m_iport = iport;
	m_eNettype = (NetType)nettype;
	//Init();
}

NetBase::~NetBase(){
	if (m_pevent)
		free(m_pevent);
	m_pevent = NULL;
	close(m_efd);
	close(m_sockfd);
	close(m_Recvpipe);
}

int NetBase::Init(){
	int sockflag;
	int buffsize = MAX_BUFFLEN;
	m_efd = epoll_create(MAXEPOLL);
	m_pevent = (struct epoll_event*)calloc(MAXEOPLL,sizeof(struct epoll_event));

	m_Recvpipe = pipe()
	struct sockaddr_in svraddr;

	memset(&hints, 0, sizeof(struct sockaddr_in));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = m_iport;
	svraddr.sin_addr = INADDR_ANY;

	if (m_eNettype == TCP_NETTYPE){		
		m_sockfd = socket(AF_INET,SOCK_STREAM,0);
		if (m_sockfd == -1)
			return ERR_SOCKCREATE;
		}
	else{
		m_sockfd = socket(AF_INET,SOCK_DGRAM);
		if (m_sockfd == -1)
			return ERR_SOCKCREATE;
		}

	if (-1 == bind(m_sockfd,(struct sockaddr*)&svraddr,sizeof(struct sockadr))
		return ERR_SOCKBIND;

	if(m_eNettype==TCP_NETTYPE && listen(m_sockfd,MAX_LISTEN))
		return ERR_SOCKLISTEN;

	//set Nonblock
	sockflag = fcntl(m_sockfd,F_GETFL,0);
	sockflag |= O_NONBLOCK;
	fcntl(m_sockfd,F_SETFL,sockflag);

	//set socket rcv/send buff length
	setsockopt(m_sockfd,SOL_SOCKET,SO_RCVBUF,(char*)&buffsize, sizeof(int));
	setsockopt(m_sockfd,SOL_SOCKET,SO_SNDBUF,(char*)&buffsize, sizeof(int));
	struct stEvent _localevent;
	SetEvents(m_sockfd,EPOLLIN,AcceptConn,1,&_localevent);
	return AddEvent(&_localevent);	
}

int NetBase::AcceptConn(int fd, int events, void* argv){
		struct sockaddr_in	accsin;
		bzero(&accsin,sizeof(accsin));
		socklen_t len = sizeof(struct sockaddr_in);

		int accfd = accept(fd,(struct sockaddr*)accsin,&len);
		if (accfd == -1)
			return ERR_SOCKACCEPT;

		if (m_epollsize == MAXEPOLL)
			return ERR_EPOLLFULL;
		SetEvent(accfd,EPOLLIN,RecvData,1,&m_stevent[accfd]);
		AddEvents(&m_stevent[accfd]);
		g_arrystfd[accfd].clientIp = accsin->sin_addr;
		g_arrystfd[accfd].clientPort = accsin->sin_port;
		return 0;
}

int NetBase::RecvData(int fd, int events, void* argv){
	struct stEvent* pev = (struct stEvent*)argv;
	int readnum = recv(pev->fd,pev->buff+pev->offset,sizeof(pev-buff)-pev->offset-1,0);
	pev->len += readnum;
	pev->offset += readnum;

	if (readnum == 0){
		close(pev->fd);
		DelEvents(pev);
	}
	else if (readnum > 0){
		pev->buff[pev->len] = '\0';
		
		bzero(g_arrystfd[pev->fd].recv,MAX_BUFFLEN);
		memcpy(g_arrystfd[pev->fd].recv,pev->buff,pev->len);
		
		//wirte shm 
		//@param g_Queenid is created from main.cpp
		stMsg queenmsg(g_Queenid);
		queenmsg.typeid = 100;
		queenmsg.fd = pev->fd;
		bzero(queenmsg.msgbuff,MAX_BUFFLEN);
		memcpy(queenmsg.msgbuff,pev->buff,pev->len);
		
		SetEvents(pev->fd,EPOLLOUT,SendData,1,pev);
		AddEvents(pev);
	}
	else{
		close(pev->fd);
		return ERR_SOCKREAD;
	}
	return 0;
}

int NetBase::SendData(int fd, int events, void* argv){
	struct stEvent* pev = (struct stEvent*)argv;
	int writenum = write(pev->fd,pev->buff+pev->offset,pev->len-pev->offset,0);

	if (writenum == 0){
		close(pev->fd);
		DelEvents(pev);
	}
	else if (writenum > 0){
		if (writenum == pev->len){
			DelEvents(pev);
			SetEvents(pev->fd,EPOLLIN,RecvData,1,pev);
			AddEvents(pev);
		}
		else{
			pev->offset += writenum;
		}
	}
	else{
		close(pev->fd);
		return ERR_SOCKWRITE;
	}
	return 0;
}

int NetBase::RecvQueen(int fd, int events, void* argv)
{
	struct stEvent* pev = (struct stEvent*)argv;
	MsgQueen queenApi;
	char queenmsgbuff[MAX_BUFFLEN];
	//这里要读一次管道 不然数据会越来越多
	/*
	readpipe()
	*/
	queenApi(pev->queenid);
	queenApi.RecvQueen(queenmsgbuff,0,RECV_TYPE,MAX_BUFFLEN);
}
void NetBase::SetEvents(int fd, int events, callback* pFuncCallback,int status, stEvent* event,int queenid){
	event->fd = fd;
	event->status = status;
	event->pcallback = pFuncCallback;
	event->lastactivetime = ::time(NULL);
	event->events = events;
	event->len = 0;
	bzero(event->buff,sizeof(event->buff));
	event->offset = 0;
	event->queenid = queenid;
}

int NetBase::AddEvents(struct stEvent* pev){
	struct epoll_event tmpev;

	tmpev.data.ptr = pev;
	tmpev.data.fd = pev->fd;
	tmpev.events = pev->events

	//epoll ctl "succ return 0 * failed return err_code"
	if (epoll_ctl(m_efd,EPOLL_CTL_ADD,pev->fd,&tmpev) !=0)
		return ERR_EPOLLCTL;

	return 0;
}

int NetBase::DelEvents(struct stEvent* pev){
	struct epoll_event tmpev;

	tmpev.data.ptr = pev;
	tmpev.data.fd = pev->fd;
	tmpev.events = pev->events

	if (epoll_ctl(m_efd,EPOLL_CTL_DEL,pev->fd,&tmpev) !=0)
		return ERR_EPOLLDEL;

	return 0;
}

int NetBase::WaitForEvents(int timeout){
	while(true){
		int fd_count = epoll_wait(m_efd,m_pevent,MAXEPOLL,timeout);
		NetLoop(fd_count);
	}
}

int NetBase::NetLoop(int eventCount){
	int i= 0;
	struct stEvent* pev;
	for(;i<eventCount;++i){
		int fd = m_pevent[i].data.fd;

	//句柄出错
	if (m_pevnet[i].events & (EPOLLHUP|EPOLLERR)){
		close(fd);
		continue;
	}
	pev = (struct stEvent*)m_pevent[i].data.ptr;
	//句柄可写
	if (m_pevent[i].events & EPOLLOUT){
		pev->pcallback(pev->fd,m_pevent[i].events,pev);
	}
	//句柄可读
	if (m_pevent[i].events & EPOLLIN){
		pev->pcallback(pev->fd,m_pevent[i].events,pev);
	}
	}
	return 0;
}

int NetBase::Tcp_Send_N(int sockfd, void *buf, int total)
{
	int nbyte,nsend;
	for(nbyte=0;nbyte<total;){
		nsend = nbtye + send(sockfd,buf+nbyte,total-nbyte,0);
		if(nsend == 0)
			return ERR_TCPSEND;
		else if((nsend+nbyte)<total){
			nbyte += nsend;
		}
		else if(nsend == -1){
			if(errno == EINTR)
				nbtye = 0;
			else if(errno == EAGAIN)
				return nbyte;
			else
				return ERR_TCPSEND;
		}
		else{
			return ERR_TCPSEND;
		}
		
	}
	return nbtye;
}
