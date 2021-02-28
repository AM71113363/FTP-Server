#include "main.h"

int net_closesocket(int *fd)
{
     int ret; 
     int error_code, opt_len;    
     if(*fd < 1)
        return ME_OK;  
    error_code = 0;
    opt_len = sizeof(error_code);
    ret=getsockopt( *fd, SOL_SOCKET, SO_ERROR, (char *)&error_code, &opt_len);
    if(ret!=0)
        return ME_OK;
    if(error_code == WSAENOTSOCK)
        return ME_OK;
    if(error_code == WSAECONNRESET)
    return ME_OK;
   if(closesocket(*fd) == SOCKET_ERROR)
      return ME_ERROR; //error
   *fd = -1;
return ME_OK;
}
//-1 error with this socket
//0 timeout
//1 ok have data
int net_isrecv(int fd,fd_set *read_fds, int sec )
{
    int ret;
    struct timeval tv; 
    if(fd <1)
      return -1;
    FD_ZERO( read_fds );
    FD_SET( fd, read_fds );
    tv.tv_sec  = sec;
    tv.tv_usec = 0;
    ret = select(fd+1, read_fds, NULL, NULL, &tv );
    if(ret == SOCKET_ERROR)
       return -1; //error
    if(FD_ISSET( fd, read_fds ))
        return 1; //ok have data
  return 0;
}

int net_send( int *fd, UCHAR *buf, UINT len)
{ 
    int ret;
    UINT sBytes = len;
    UCHAR *p= buf;
    while(1)
    {
        ret = send( *fd, p, sBytes ,0);
        if( ret > 0 )
        {
            p += ret;
            sBytes -= ret;
            if(sBytes < 1 )
            {
                ret = ME_OK;        
                break; //ok
            }
        }
        else
        {
            ret = ME_ERROR;
            break;
        }
    }
  return ret;
}


static USHORT net_htons( int port )
{
    UCHAR buf[4];

    buf[0] = (UCHAR)(port >> 8 );
    buf[1] = (UCHAR)(port);
    buf[2] = buf[3] = 0;

    return( *(USHORT*) buf );
}


int net_bind( int *fd, int port ,int nListen)
{
    int n = 1;
    struct sockaddr_in server_addr;
    *fd = socket(AF_INET, SOCK_STREAM, 6);
    
    if(*fd < 1 )
        return ME_ERROR; //error

    setsockopt( *fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &n, sizeof( n ) );

    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = net_htons( port );

    if( bind( *fd, (struct sockaddr *) &server_addr, sizeof( server_addr ) ) < 0 )
    {
        closesocket( *fd );
        return ME_ERROR; //error
    }
    
    if( listen( *fd, nListen) != 0 )
    {
        closesocket( *fd );
        return ME_ERROR; //error
    }  
    return ME_OK; //ok
}


int net_accept( int bind_fd, int *client_fd)
{
    struct sockaddr_in client_addr;
    int ret;
    int n = (int) sizeof( client_addr );
    
    *client_fd = -1;    
    *client_fd = accept( bind_fd, (struct sockaddr *) &client_addr, &n );
    if( *client_fd < 1 )
    {
        ret = ME_ERROR; //error
    }
    else
    {
        ret = ME_OK; //ok
    }
    return ret;
}

