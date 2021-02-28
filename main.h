#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>

#define ISCHECKED(_h_)           SNDMSG(_h_,BM_GETCHECK,0,0)==BST_CHECKED
#define WSV        WS_CHILD|WS_VISIBLE
#define SETF(_h_) SendMessage(_h_, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0))

#define ME_OK       1
#define ME_ERROR   -1
#define ID_START   1001
#define ID_STOP   1002

#define YES 1
#define NO  0

#define BSIZE 1024

HINSTANCE ins;
HWND logger;
HWND hWnd;

HWND hUsername;
HWND hPassword;
HWND hAuth;
HWND hDelete;
HWND hStart;
HWND hStop;

#define SMS(_x_) net_send( &_x_->connection, _x_->msg, strlen(_x_->msg))

typedef struct _sSocket
{
    int sock;
    UCHAR msg[128];
}TSOCKET;

int Mainsock;

int net_bind( int *fd, int port ,int nListen);
int net_accept( int bind_fd, int *client_fd);
int net_closesocket(int *fd);
int net_send( int *fd, unsigned char *buf,unsigned int len);


typedef struct tpass_
{
     int c;
     UCHAR username[101];
     UCHAR password[101];
}TPASS;


typedef struct commnad_
{   
    UCHAR command[5];
    UCHAR arg[MAX_PATH];
    UCHAR CurrDir[MAX_PATH];
    UCHAR Temp[MAX_PATH];
    DWORD offset;
    UINT logged_in;//Is user loggd in?
    UINT username_ok;//Is this username allowed?
    UCHAR username[101];
    UCHAR password[101];
    UCHAR *msg;
    int connection;
    int sock_pasv; 
    int mode;     
}CLIENT;


#define DontExist   0
#define ThisIsFile      1
#define ThisIsDir       2

#define NORMAL 0
#define SERVER 1

int f_ABOR(CLIENT *,TSOCKET *);
int f_CLNT(CLIENT *,TSOCKET *);
int f_FEAT(CLIENT *,TSOCKET *);
int f_HELP(CLIENT *,TSOCKET *);
int f_OPTS(CLIENT *,TSOCKET *);
int f_PASV(CLIENT *,TSOCKET *);
int f_PASS(CLIENT *,TSOCKET *);
int f_QUIT(CLIENT *,TSOCKET *);
int f_SYST(CLIENT *,TSOCKET *);
int f_TYPE(CLIENT *,TSOCKET *);
int f_USER(CLIENT *,TSOCKET *);
int f_CDUP(CLIENT *,TSOCKET *);
int f_CWD (CLIENT *,TSOCKET *);
int f_DELE(CLIENT *,TSOCKET *);
int f_LIST(CLIENT *,TSOCKET *);
int f_MDTM(CLIENT *,TSOCKET *);
int f_MKD (CLIENT *,TSOCKET *);
int f_MLST(CLIENT *,TSOCKET *);
int f_PWD (CLIENT *,TSOCKET *);
int f_REST(CLIENT *,TSOCKET *);
int f_RETR(CLIENT *,TSOCKET *);
int f_RMD (CLIENT *,TSOCKET *);
int f_RNFR(CLIENT *,TSOCKET *);
int f_RNTO(CLIENT *,TSOCKET *);
int f_SIZE(CLIENT *,TSOCKET *);
int f_STOR(CLIENT *,TSOCKET *);
int f_NOOP(CLIENT *,TSOCKET *);

typedef int (*fnc)(CLIENT *,TSOCKET *);

typedef struct CMD_FUNC_
{
     UCHAR *cmd;
     fnc func;
}CMD_FUNC;

static CMD_FUNC CMDCOMMANDS[]={
{ "ABOR", f_ABOR},
{ "CLNT", f_CLNT},
{ "FEAT", f_FEAT},
{ "HELP", f_HELP},
{ "OPTS", f_OPTS},
{ "PASV", f_PASV},
{ "PASS", f_PASS},
{ "QUIT", f_QUIT},
{ "SYST", f_SYST},
{ "TYPE", f_TYPE},
{ "USER", f_USER},
{ "CDUP", f_CDUP},
{ "CWD",  f_CWD },
{ "DELE", f_DELE},
{ "LIST", f_LIST},
{ "MDTM", f_MDTM},
{ "MKD",  f_MKD },
{ "MLST", f_MLST},
{ "PWD",  f_PWD},
{ "REST", f_REST},
{ "RETR", f_RETR},
{ "RMD",  f_RMD},
{ "RNFR", f_RNFR},
{ "RNTO", f_RNTO},
{ "SIZE", f_SIZE},
{ "STOR", f_STOR},
{ "NOOP", f_NOOP},
{ NULL,NULL}
};


