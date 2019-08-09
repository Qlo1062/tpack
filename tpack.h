#ifndef      TPACK_H_
#define      TPACK_H_

#include "define.h"

//  通信包结构
#define     TPACK_HEAD_SIZE    512				//	头信息先设置最大
#define     TPACK_HEAD_DATA_SIZE	64			//	标准大小
#define     TPACK_DATA_SIZE    8192
#define     TPACK_SIZE         8192 + 512
#define     TPACK_HEAD_LINE_TAG "\r\n"
#define     TPACK_HEAD_TAG      "\r\n106258\r\n"  //  头结束符
#define     TPACK_HEAD_TAG_SIZE strlen(TPACK_HEAD_TAG)
#define     TPACK_DATA_TAG      "\r\n4641\r\n"  //  数据结束符
#define     TPACK_DATA_TAG_SIZE strlen(TPACK_DATA_TAG)

//	接收包的缓冲区建议大小
#define     TPACK_CACHE_SIZE	TPACK_SIZE * 2	//	包的缓冲区建议长度为 一个包的两倍大小

//  包命令
typedef     unsigned int    TPackCmd;
#define     TPACK_CMD_EMPTY         0x1 //  空命令,做keep
#define     TPACK_CMD_SEND          0x2     //  T端数据传到Client
#define     TPACK_CMD_T_CLOSE       0x4     //  target要断开了
#define     TPACK_CMD_T_T_CLOSE     0x8     //  target下的目标以关闭,S判断后关闭,指向的socket客户端
#define     TPACK_CMD_T_LOGIN       0x10    //  target请求登陆,验证成功后返回user_token 和 task列表
#define     TPACK_CMD_5      0x20
#define     TPACK_CMD_6      0x40
#define     TPACK_CMD_7      0x80

//  包Form
#define     TPACK_FORM_SERVER   "Server"
#define     TPACK_FORM_CLIENT   "Client"
#define     TPACK_FORM_TARGET   "Target"

//	包格式
typedef struct ST_TPACK_INFO
{
    TPackCmd    cmd;
	char	    form[TPACK_HEAD_DATA_SIZE];
	uint		sock_cli;	//	服务器传回的客户端sock标识
	uint		h_data_len;

	char*	phead;
	uint		phead_len;
    
	char*	pdata;
	uint		pdata_len;

	char*	pstr;		//	序列化后的数据
	uint	pstr_len;
} *TPack;

TPack tpack_new( TPackCmd cmd, char* form, uint sock, char* data, size_t size);

void tpack_update_head(TPack tpack);

char* tpack_tostr(TPack tpack);

void tpack_free(TPack tpack);

TPack tpack_parse(char* str, uint str_len, char **end);

#endif
