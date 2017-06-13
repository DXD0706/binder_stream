
static vector<int> g_vec_desc;
static int g_last_desc = 0;

int dfes_binder_init( char *context )
{
    struct sockaddr_un address;
    int sock;
    size_t addrLength;

    if ( ( sock = socket( PF_UNIX, SOCK_STREAM, 0 ) ) < 0 ) {
        return -1;
    }

    address.sun_family = AF_UNIX;
    strcpy( address.sun_path, "./sample-socket" );

    addrLength = sizeof( address.sun_family ) + strlen( address.sun_path );

    if ( connect( sock, ( struct sockaddr * ) &address, addrLength ) != 0 ) {
        close( sock );
        return -1;
    }
    return sock;

}

int dfes_binder_uninit( int desc )
{
    return close( desc );
}

int dfes_binder_register( int desc, uint32 msgno )
{

}

int dfes_binder_unregister( int desc, uint32 msgno )
{

}



int dfes_binder_send_message( int desc, uint32 msgno, uint8 *buf, size_t len )
{
    uint8  msgbuf[MAX_DFES_BINDER_MSG_LEN];


    msgbuf[0] = 0xff;
    msgbuf[1] = 0xff;
    msgbuf[2] = ( len + 2 ) % 256;
    msgbuf[3] = ( len + 2 ) / 256;
    msgbuf[4] = msgno % 256;
    msgbuf[5] = msgno / 256;
    memcpy( &msgbuf[6], buf, len );

    len += 6;
    int consumed = 0;
    while( consumed < len ) {
        int n = send( desc, &msgbuf[consumed], len - consumed, 0 );
        if ( n < 0 ) {
            return -1;
        }
        consumed += n;
    }
    return 0;
}

int dfes_binder_send_message( int desc, dfes_binder_message_t *msg )
{
    if ( desc < 0 || msg == NULL ) {
        errno = EINVAL;
        return -1;
    }
    return dfes_binder_send_message( desc, msg->msgno, msg->buf, msg->len );
}


int dfes_binder_recv_message( int desc, dfes_binder_message_t *msg )

{
    int sock = desc;
    uint8 buf[8192];
    raw_msg_half_t half;
    raw_msg_t      msg;
    memset( buf, 0x00, sizeof( buf ) );

    while ( 1 ) {
        int n = recv( sock, buf, sizeof( buf ), 0 );
        if ( n < 0 ) {
            break;
        }

        if ( n == 0 ) {
            break;
        }

        int offset = 0;

        while( offset < n ) {
            int consumed = split_message( &buf[offset], n - offset, &half, &msg );

            //printf("consumed %d\n",consumed);
            if ( consumed < 0 ) {
                return NULL;
            }

            if ( half.len > 0 ) {
                break;
            }

            if ( msg.len > 0 ) {
                if ( msg.msgno == 0 ) {
                    register_message( sock, &msg );
                }

            }

            offset += consumed;

        }

        g_len += n;

    }

    close( sock );
    return NULL;
}

