#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dfes_typeint.h"
#include "dfes_binder.h"
#include <map>
#include <set>

using namespace std;
long g_len = 0;

map<uint32, set<int> > g_map_msgno;
void *t_counter( void *p )
{
    long len = 0;

    while ( 1 ) {
        sleep( 1 );
        long len2 = g_len;
        printf( "speed %ld MB/s %ld\n", ( len2 - len ) / 1024 / 1024, len2 );
        len = len2;
    }

    return NULL;
}

int split_message( uint8 *buf, uint32 len, raw_msg_half_t *half, raw_msg_t *msg )
{
    if ( buf == NULL || half == NULL || msg == NULL || len == 0 ) {
        errno = EINVAL;
        return -1;
    }

    if ( half->len > 0 ) {
        if ( half->len == 1 ) {
            if ( len == 1 ) {
                half->buf[half->len] = buf[0];
                half->len += 1;
                return 1;
            } else {
                uint32 msg_body_len = buf[0] + buf[1] * 256;

                if ( msg_body_len < 3 ) {
                    return -1;
                }

                if ( len < msg_body_len + 2 ) {
                    memcpy( &half->buf[half->len], buf, len );
                    half->len += len;
                    return len;
                } else {
                    msg->buf[0] = half->buf[0];
                    half->len = 0;
                    memcpy( &msg->buf[1], buf, msg_body_len + 2 );
                    msg->len = msg_body_len + 3;
                    msg->msgno = msg->buf[3] + msg->buf[4] * 256;
                    return msg_body_len + 2;
                }
            }
        } else if ( half->len == 2 ) {
            uint32 msg_body_len = half->buf[1] + buf[0] * 256;

            if ( msg_body_len < 3 ) {
                return -1;
            }

            if ( len < msg_body_len + 1 ) {
                memcpy( &half->buf[half->len], buf, len );
                half->len += len;
                return len;
            } else {
                msg->buf[0] = half->buf[0];
                msg->buf[1] = half->buf[1];
                half->len = 0;
                memcpy( &msg->buf[2], buf, msg_body_len + 1 );
                msg->len = msg_body_len + 3;
                msg->msgno = msg->buf[3] + msg->buf[4] * 256;
                return msg_body_len + 1;
            }
        } else {
            uint32 msg_body_len = half->buf[1] + half->buf[2] * 256;
            uint32 left_len = msg_body_len + 3 - half->len;

            if ( len < left_len ) {
                memcpy( &half->buf[half->len], buf, len );
                half->len += len;
                return len;
            } else {
                memcpy( msg->buf, half->buf, half->len );
                memcpy( &msg->buf[half->len], buf, left_len );
                half->len = 0;
                msg->len = msg_body_len + 3;;
                return left_len;
            }
        }
    } else {
        if ( buf[0] != 0xff ) {
            return -1;
        }

        if ( len < 3 ) {
            memcpy( half->buf, buf, len );
            half->len = len;
            return len;
        } else {
            uint32 msg_body_len = buf[1] + buf[2] * 256;

            if ( len < msg_body_len + 3 ) {
                memcpy( half->buf, buf, len );
                half->len = len;
                return len;
            } else {
                memcpy( msg->buf, buf, msg_body_len + 3 );
                msg->len = msg_body_len + 3;
                return msg_body_len + 3;
            }
        }
    }
}

int register_message( int sock, raw_msg_t *msg )
{
    if ( sock < 0 || msg == NULL ) {
        errno = EINVAL;
        return -1;
    }

    if ( msg->msgno != 0 ) {
        errno = EINVAL;
        return -1;
    }

    if ( msg->len != 5 ) {
        errno = EINVAL;
        return -1;
    }

    if ( msg->buf[0] != 0xff && msg->buf[1] != 0xff ) {
        return -1;
    }

    if ( msg->buf[2] != 0x01 ) {
        return -1;
    }

    uint32 msgno = msg->buf[3] + msg->buf[4] * 256;

    map<uint32, set<int> >::iterator it_map = g_map_msgno.find( msgno );

    if ( it_map == g_map_msgno.end() ) {
        it_map->second.insert( sock );
        return 0;
    } else {
        set<int> set_sock;
        set_sock.insert( sock );
        g_map_msgno.insert( map<uint32, set<int> >::value_type( msgno, set_sock ) );
    }

    return 0;
}
int dispatch_message( raw_msg_t *msg )
{
    if ( msg == NULL ) {
        errno = EINVAL;
        return -1;
    }

    map<uint32, set<int> >::iterator it_map = g_map_msgno.find( msg->msgno );

    if ( it_map == g_map_msgno.end() ) {
        return -1;
    }

    for ( set<int>::iterator it_set = it_map->second.begin(); it_set != it_map->second.end(); it_set++ ) {
        send( *it_set, msg->buf, msg->len, 0 );
    }

    return 0;
}
void *t_recv_message( void *p )
{
    int sock = *( int * )p;
    free( p );

    uint8 buf[8192];
    raw_msg_half_t half;
    raw_msg_t      msg;
    memset( buf, 0x00, sizeof( buf ) );

    while ( 1 ) {
        int n = recv( sock, buf, sizeof( buf ), 0 );

        //printf("recv return %d\n",n);
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

int main( int argc, char **argv )
{
    struct sockaddr_un address;
    int sock, conn;
    socklen_t addrLength;
    pthread_t tid;

    if ( ( sock = socket( PF_UNIX, SOCK_STREAM, 0 ) ) < 0 ) {
        perror( "socket" );
        return 0;
    }

    unlink( "./sample-socket" );
    address.sun_family = AF_UNIX;       /* Unix domain socket */
    strcpy( address.sun_path, "./sample-socket" );

    addrLength = sizeof( address.sun_family ) + strlen( address.sun_path );

    if ( bind( sock, ( struct sockaddr * ) &address, addrLength ) ) {
        perror( "bind" );
        return 0;
    }

    if ( listen( sock, 5 ) ) {
        perror( "listen" );
        return 0;
    }

    pthread_create( &tid, NULL, t_counter, NULL );

    while ( 1 ) {
        conn = accept( sock, ( struct sockaddr * ) &address, &addrLength );

        if ( conn < 0 ) {
            continue;
        }

        int *pconn = ( int * )malloc( sizeof( int ) );
        *pconn = conn;
        pthread_create( &tid, NULL, t_recv_message, ( void * )pconn );
    }

    close( sock );
    return 0;
}

