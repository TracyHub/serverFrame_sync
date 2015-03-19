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


#endif

