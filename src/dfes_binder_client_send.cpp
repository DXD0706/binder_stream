#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main( int argc, char **argv )
{
    struct sockaddr_un address;
    int sock;
    size_t addrLength;

    if ( ( sock = socket( PF_UNIX, SOCK_STREAM, 0 ) ) < 0 ) {
        perror( "socket" );
        return 0;
    }

    address.sun_family = AF_UNIX;    /* Unix domain socket */
    strcpy( address.sun_path, "./sample-socket" );

    addrLength = sizeof( address.sun_family ) + strlen( address.sun_path );

    if ( connect( sock, ( struct sockaddr * ) &address, addrLength ) != 0 ) {
        perror( "connect" );
        return 0;
    }

    unsigned char buf[8192];
    memset( buf, 0x00, sizeof( buf ) );
    buf[0] = 0xff;
    buf[1] = 1000 % 256;
    buf[2] = 1000 / 256;
    buf[3] = 100 % 256;
    buf[4] = 100 / 256;

    while ( true ) { // test
        send( sock, buf, 1003, 0 );
    }

    close( sock );

    return 0;
}

