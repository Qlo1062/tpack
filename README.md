# tpack
解决tcp沾包的一种方法，带有定制化头信息 
A way to resolve TCP packets with custom headers

// TPACK::装包
        TPack tpack = tpack_new( TPACK_CMD_SEND, TPACK_FORM_SERVER, sock, recv_buf, recvSizeOk );
        tpack_tostr( tpack );
        
            //发送tpack->pstr
        tpack->pstr, tpack->pstr_len
        
        tpack_free( tpack );


//  不断接收到缓冲区中再解包
// TPACK::解包
    char* tpack_data_end;   //  解析成功后，结尾数据位置，结束符在内
    TPack tpack = tpack_parse( link->recvCache, link->recvLen, &tpack_data_end );
    if( tpack == NULL ){    //  拆包失败，继续接收
        printf("target_in_thr: tpack parse failed, wait next recv\n");
        
        set_event_et( link->epoll_fd, EPOLLIN, link->sock );
        return NULL;
    }
 
// TPACK::缓冲区向前移动, 如果尾部相等，直接清空
    int move_len = (link->recvCache + link->recvLen) - tpack_data_end;
    if( move_len == 0 ){
        memset( link->recvCache, 0, TPACK_CACHE_SIZE );
        link->recvLen = 0;
        printf("target_in_thr: recvCache memset\n");
    }
    else {
        int move_size = tpack_data_end - link->recvCache;   //  结束符减去指针就是长度
        memmove( link->recvCache, tpack_data_end, move_size );
        link->recvLen = move_size;
        printf("target_in_thr: recvCache move forward, recvCache: %p, data_end: %p, move size: %d\n", link->recvCache, tpack_data_end, move_size);
    }


