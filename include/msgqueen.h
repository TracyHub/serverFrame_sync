#ifndef MSG_QUEEN_HEAD
#define MSG_QUEEN_HEAD
#include "public_define.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>

class MsgQueen{
	public:
		MsgQueen(){};
		~MsgQueen(){};
		inline int InitMsgQueen(int queenid = 0){
			if (queenid)
				m_queenid = queenid;
			else
				if ((m_queenid = msgget(IPC_PRIVATE, IPC_CREAT))<0)
					return ERR_MSGCREATE;

			return 0;
		}
		int SendMsg(void* sndbuff,int flag);
		int RecvMsg(void * recvbuff, int flag, unsigned int typeid, int len);
	public:
		int m_queenid;//消息队列id
		
};
#endif
