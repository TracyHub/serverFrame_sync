#ifndef CALL_BACK_HEAD
#define CALL_BACK_HEAD

#include "stdio.h"

typedef int (*pFuncHandleInit) (char argc,char* argv);
typedef int (*pFuncHandleProc) (char* buf);
typedef int (*pFuncHandleFin) (char* buf);
typedef int (*callback) (int fd, int event, void* argv);


typedef struct _CallBack{
	void*				paddress;
	pFuncHandleInit		handleInit;
	pFuncHandleProc		handleProc;
	pFuncHandleFin		hanldeFin;
}callback;

extern callback g_dll;


#endif
