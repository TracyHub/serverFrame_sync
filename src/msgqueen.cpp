#include "msgqueen.h"

int MsgQueen::SendMsg(void * sndbuff, int flag, int len){
	  if ( (msgsnd(m_queenid, sndbuff, len, flag )) < 0 ){   /*����Ϣ�����з�����Ϣ*/
        return ERR_MSGSEND;

		return 0;
    }
}

int MsgQueen::RecvMsg(void * recvbuff, int flag, unsigned int typeid, int len){
	if ( (msgrcv(m_queenid, recvbuff, len, typeid, flag)) < 0 ){ /*����Ϣ����ȡ��Ϣ*/
		return ERR_MSGRECV;

	return 0;
	}
}

