#include <stdlib.h>
#include <malloc.h>
#include "netbase.h"
#include "call_back.h"
#include "public_define.h"
#include "msg_queen.h"

char* g_processName;
char* confFile;
char* dllFile;
char* logFile;

extern callback g_dll;

void ShowUsage(char argc,char* argv[]){
	char process[64];
	process = argv[0];
	printf("usage:%s configFile[*.xml/json] dll[*.so]");
	exit(0);
}

void ParseParams(char argc,char* argv[]){
	g_processName 	= strdup(argv[0]);
	confFile 		= strdup(argv[1]);
	dllFile 		= strdup(argv[2]);
	logFile 		= strdup(argv[3]);
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

void CloseDll(){
	if (g_dll.paddress)
		dlclose(g_dll.paddress);
	g_dll.paddress = NULL;
}

void InitConfig(){
	//empty
}

void InitQueenMsg(){
	//empty
}

void InitNetWork(){
	//empty
}

void InitLog(){
	//empty
}

int main(char argc,char* argv[])
{
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

	//Init queen msg
	InitQueenMsg();

	//Init Network---default tcp
	InitNetWork();

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

	}
	else{
		//child process get msg from queen,and deal it
	}
	return 0;
}