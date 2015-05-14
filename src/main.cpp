#include <stdlib.h>
#include <malloc.h>
#include "netbase.h"
#include "call_back.h"
#include "public_define.h"
#include "msg_queen.h"

#if 1
define DEAULT_PORT 18000
#endif

#define 
using namespace NetBaseSpace;
char* g_processName;
char* confFile;
char* dllFile;
char* logFile;
char g_data2client[MAX_BUFFLEN];

int worker_in_coming[2] = {-1,-1};

extern callback g_dll;
MsgQueen g_msgqueen;
MsgQueen g_recvqueen;
NetBase g_netapi(DEAULT_PORT,TCP_NETTYPE);

void ShowUsage(char argc,char* argv[]){
	char process[64];
	process = argv[0];
	printf("usage:%s configFile[*.xml/json] dll[*.so]");
	exit(0);
}

void Working_in_coming_create()
{
	if (pipe(worker_in_coming) == -1){
		LOG_ERROR("pipe create error,%d",errno);
		exit(1);
	}
	fcntl(worker_in_coming[0],F_SETFL,O_NONBLOCK|O_RDONLY);
	fcntl(worker_in_coming[1],F_SETFL,O_NONBLOCK|O_WRONLY);
}
void ParseParams(char argc,char* argv[]){
	g_processName 	= strdup(argv[0]);
	confFile 		= strdup(argv[1]);
	dllFile 		= strdup(argv[2]);
	logFile 		= strdup(argv[3]);
}

void DoWork()
{
	while(1){
		int iRet;
		char msgbuff[MAX_BUFFLEN];
		stMsg _localmsg;
		iRet = g_msgqueen.RecvMsg(msgbuff,0,MAX_BUFFLEN,RECV_TYPE);
		if (iRet < 0){
			LOG_ERROR("msg recv ERROR");
		}
		stMsg* pmsg = (stMsg*)msgbuff;
		if (dll.handleProc)
			iRet = dll.handleProc(pmsg->msgbuff,pmsg->len,_localmsg.msgbuff,&_localmsg.len);

		if (iRet){
			LOG_ERROR("worker handleProc error");
		}
		
		_localmsg.typeid = SEND_TYPE;
		_localmsg.fd = pmsg->fd;

		g_recvqueen.SendMsg((void*)&_localmsg,0,sizeof(stMsg)+_localmsg.len);
	}
}

void WorkRun()
{
	int i = 0;
	for(;i<g_processName;i++){
		pid_t pid = fork();
		if(pid == 0){
			DoWork();
		}
	}
}
void InitDll(){
	char* error;
	g_dll.paddress = dlopen(dllFile,RTLD_NOW);
	if ((error=dlerror())!=NULL){
		LOG_ERROR("dlopen fail,filename:%s,errno:%d",dllFile,errno);
		exit(0);
	}
	//init callback function
	DLL_GETFUNCADDR(g_dll,g_dll.handleInit,pFuncHandleInit,"handleInit");
	DLL_GETFUNCADDR(g_dll,g_dll.handleProc,pFuncHandleProc,"handleProcess");
	DLL_GETFUNCADDR(g_dll,g_dll.handleFin,pFuncHandleFin,"handleFinal");
}

void LoopRecvFromWorker()
{
	char sendBuff[MAX_BUFFLEN];
	while(stop){
		int iRet = g_recvqueen.RecvMsg(sendBuff,0,SEND_TYPE,MAX_BUFFLEN);
		if (iRet == 0){
			stMsg* _localmsg = (stMsg*)sendBuff;
			iRet = g_netapi.Tcp_Send_N(_localmsg->fd,_localmsg->msgbuff,_localmsg->len);
			if (iRet)
				LOG_ERROR("Tcp_Send_N error,iRet:%d",iRet);
		}
	}
}

void CloseDll(){
	if (g_dll.paddress)
		dlclose(g_dll.paddress);
	g_dll.paddress = NULL;
}

void InitConfig(){
	//empty
}

int InitQueenMsg(){
	if (g_recvqueen.Init() || g_msgqueen.Init())
		return ERR_INITQUEEN;
	//return g_msgqueen.Init();
}


void InitNetWork(){
	return g_netapi.Init();
}

void InitLog(){
	//empty
}

void RunAccept()
{
	g_netapi.WaitForEvents(500);
}

int main(char argc,char* argv[])
{
	int iRet;
	pid_t pid;
	//show usage
	if (argc < 4 || strcmp(argv[1],"-h") | strcmp(argv[1],"-help"))
		ShowUsage(argc,argv);

	//Init Log
	InitLog();

	//parse params
	ParseParams(argc,argv);

	//init dll 
	InitDll();

	//Init Config file
	InitConfig();

	//creat worker_in_coming pipe
	Working_in_coming_create();
	//Init queen msg
	if ((iRet = InitQueenMsg()) != 0){
		LOG_ERROR("init msgqueen err,iret:%d",iRet);
		return iRet;
	}


	//Init workprocess
	if(g_dll.handleInit){
		int iret = g_dll.handleInit(argc,argv);
		if(iret) exit(0);
	}

	//fork child process---worker
	if ((pid = fork()) < 0 ){
		LOG_ERROR("main fork error");
	}
	else if(pid > 0){
		//parent process listen port and send reqmsg to queen
		//Init Network---default tcp
		struct stEvent _localevent;
		g_netapi.SetEvent(worker_in_coming[0],EPOLLIN,g_netapi.RecvQueen,1,&_localevent,g_recvqueen.m_queenid);

		if ((iRet = g_netapi.AddEvent(&_localevent){
			LOG_ERROR("main AddEvent err,iRet:%d",iRet);
			return iRet;
		}

		if ((iRet = InitNetWork() != 0){
			LOG_ERROR("init network err,iret:%d",iRet);
			return iRet;
		RunAccept();
		}
		LoopRecvFromWorker();
	}
	else{
		//child process get msg from queen,and deal it
		WorkRun();
	}
	return 0;
}