#include "stdafx.h"
#include "com.h"
#include "tpack.h"

/**
*   通信装包和拆包
*/

//  创建包，目前是直接申请最大空间，后期应该需要先创建头、创建数据 再合成包
TPack tpack_new( TPackCmd cmd, char* form, uint sock, char* data, size_t size){
	TPack tpack = (TPack) malloc( sizeof(struct ST_TPACK_INFO) );
	memset( tpack, 0, sizeof(struct ST_TPACK_INFO) );
    tpack->cmd = cmd;
    strncpy( tpack->form, form, strlen(form) );
    tpack->sock_cli = sock;
    
	//	为头和数据区申请空间标准大小空间
	tpack->phead = (char *) malloc( TPACK_HEAD_SIZE );
	memset( tpack->phead, 0, TPACK_HEAD_SIZE );
	tpack->pdata = (char *) malloc( TPACK_DATA_SIZE );
	memset( tpack->pdata, 0, TPACK_DATA_SIZE );

	memcpy( tpack->pdata, data, size);
	tpack->pdata_len = strlen( tpack->pdata );

	tpack_update_head(tpack);
    
    return tpack;
}
//	更新头信息，不含头结束符
void tpack_update_head(TPack tpack)
{
    sprintf( tpack->phead, "Command: %d%sFrom: %s%sSocket-Clint: %d%sData-Size: %d", 
                tpack->cmd, TPACK_HEAD_LINE_TAG,  
                tpack->form, TPACK_HEAD_LINE_TAG, 
                tpack->sock_cli, TPACK_HEAD_LINE_TAG,
				tpack->pdata_len);
	tpack->phead_len = strlen( tpack->phead );
	return;
}
//	释放 TPack
char* tpack_tostr(TPack tpack)
{
	tpack_update_head(tpack);

	tpack->pstr = (char *) malloc( TPACK_SIZE );
	memset( tpack->pstr, 0, TPACK_SIZE );

	//	marger, 使用strlen计算长度
	memcpy( tpack->pstr, tpack->phead, tpack->phead_len );
	memcpy( tpack->pstr + strlen(tpack->pstr), TPACK_HEAD_TAG, TPACK_HEAD_TAG_SIZE );
	memcpy( tpack->pstr + strlen(tpack->pstr), tpack->pdata, tpack->pdata_len );
	memcpy( tpack->pstr + strlen(tpack->pstr), TPACK_DATA_TAG, TPACK_DATA_TAG_SIZE );

	tpack->pstr_len = tpack->phead_len + TPACK_HEAD_TAG_SIZE + tpack->pdata_len + TPACK_DATA_TAG_SIZE;

    return tpack->pstr;
}

void tpack_free(TPack tpack){
	if( tpack == NULL ) return;
    
	if( tpack->phead != NULL ) free( tpack->phead );
	if( tpack->pdata != NULL ) free( tpack->pdata );
	if( tpack->pstr != NULL ) free( tpack->pstr );
    
	memset( tpack, 0, sizeof(struct ST_TPACK_INFO));
    free( tpack );
}
//	在head字符串中查找对应数据
char *tpack_head_get_val(char* str, int str_len, char* key, char *val, int val_size)
{
    memset( val, 0, val_size );
	char* str_end = str + str_len;
    char* key_start = memstr( str, str_len, key);
	if( key_start == NULL ){
		return NULL;
	}

	char* end = memstr( key_start, ( str_end - key_start ), TPACK_HEAD_LINE_TAG);
	if( end == NULL ){  //  必须找到行尾
		return NULL;
	}

    char* key_start_i = key_start + strlen(key) + 1;    //  开始字符串位置
    if( (end - key_start_i) < val_size )
        val_size = end - key_start_i;

	strncpy( val, key_start_i, val_size );
    val = trim( val );
    //printf("key_start and end exist, val_size:%d\n", val_size);
	return val;
}
void tpack_parse_head(TPack tpack){
	char headVal[TPACK_HEAD_DATA_SIZE];
	memset( headVal, 0, TPACK_HEAD_DATA_SIZE );

	tpack_head_get_val( tpack->phead, tpack->phead_len, "Form", headVal, TPACK_HEAD_DATA_SIZE);
	memcpy( tpack->form, headVal, strlen(headVal) );
	
	memset( headVal, 0, TPACK_HEAD_DATA_SIZE );
	tpack_head_get_val( tpack->phead, tpack->phead_len, "Socket-Clint", headVal, TPACK_HEAD_DATA_SIZE);
	tpack->sock_cli = atoi( headVal );
	
	memset( headVal, 0, TPACK_HEAD_DATA_SIZE );
	tpack_head_get_val( tpack->phead, tpack->phead_len, "Data-Size", headVal, TPACK_HEAD_DATA_SIZE);
	tpack->h_data_len = atoi( headVal );

	return;
}
//	如果找到返回
TPack tpack_parse(char* str, uint str_len, char **end)
{
	//	通过字符串找到结束符
	char* str_end = str + str_len;	//	数据的结尾
	char* head_start = str;
	char* data_start = str;
    char* head_tag = memstr( str, str_len, TPACK_HEAD_TAG );	//	结束符标记
	char* data_tag = memstr( str, str_len, TPACK_DATA_TAG );	//	结束符标记

	if( head_tag == NULL ){
		printf("TPACK unpack failed,head_tag is NULL\n");
		return NULL;
	}	
	else if( data_tag == NULL ){
		printf("TPACK unpack failed,data_tag is NULL\n");
		return NULL;
	}
	else if( data_tag < head_tag ) {
		//	原因1：留有未处理的data_tag
        printf("TPACK unpack failed,data_tag < head_tag error, 留有未处理的data_tag\n");
        //	设置 head_start 去掉前残余data的位置
		head_start = data_tag + TPACK_DATA_TAG_SIZE;
		//	重新在 str 搜索 data_tag 位置, head-start 需要str
		data_tag = memstr( head_start, ( str_end - head_start), TPACK_DATA_TAG );
		if( data_tag == NULL )
			printf("TPACK unpack failed,data_tag is NULL (2) \n");
			return NULL;
	}
	data_start = head_tag + TPACK_HEAD_TAG_SIZE;

	//	初始化一个空TPack信息
	TPack Tpack = tpack_new( TPACK_CMD_EMPTY, "", 0, "", 0 );

	//	找到的 头 和 内容区, 两指针想减即是之间字节大小
	Tpack->phead_len = head_tag - str;
	Tpack->pdata_len = data_tag - data_start;
	memcpy( Tpack->phead, head_start, Tpack->phead_len );
	memcpy( Tpack->pdata, data_start, Tpack->pdata_len );

	//	解析头
	tpack_parse_head( Tpack );

	*end = data_tag + TPACK_DATA_TAG_SIZE;	//	返回结束的标记位置
	
	return Tpack;
}
