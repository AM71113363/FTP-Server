#include "main.h"


int f_ABOR(CLIENT *cmd,TSOCKET *S)
{
     if(cmd->logged_in)
     {
          cmd->msg = "225 Connessione aperta; Nessuna azione in corso.\r\n";
     }
     else
     {
          cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
     }  
  return SMS(cmd);
}

int f_CLNT(CLIENT *cmd,TSOCKET *S)
{
     if(cmd->logged_in)
     {
           cmd->msg = "200 Noted.\r\n";
     }
     else
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
     }
  return SMS(cmd);
}


int f_FEAT(CLIENT *cmd,TSOCKET *S)
{
    cmd->msg = "211-Estensioni supportati:\r\n MDTM\r\n CLNT\r\n SIZE\r\n REST STREAM\r\n PASV\r\n MLST Type*;Size*;Modify*;\r\n211 Fine.\r\n";                   
    return SMS(cmd);
}


int f_HELP(CLIENT *cmd,TSOCKET *S)
{
    int i,n,ret;
    cmd->msg = "214-I seguenti comandi vengono riconosciuti.\r\n";

    ret = SMS(cmd);                         
    if(ret == ME_ERROR)
       return ret;
    i=0;
    while(CMDCOMMANDS[i].cmd!=NULL)
    {
        n = i+1;                     
        cmd->msg = "    ";
        ret = SMS(cmd);                         
        if(ret == ME_ERROR)
           return ret;                    
        cmd->msg = CMDCOMMANDS[i].cmd;
        ret = SMS(cmd);                         
        if(ret == ME_ERROR)
           return ret;                                    
        if((n % 9) == 0)
        {
           cmd->msg = "\r\n";
           ret = SMS(cmd);                         
           if(ret == ME_ERROR)
               return ret;               
        }
        i++;
     }
    if((n % 9) == 0)
    {
       cmd->msg = "214 Ciao! Buona giornata.\r\n";
    }
    else
    {
       cmd->msg = "\r\n214 Ciao! Buona giornata.\r\n";
    }
 return SMS(cmd);
}

int f_OPTS(CLIENT *cmd,TSOCKET *S)
{
     if(cmd->logged_in)
     {
         cmd->msg = "200 LOL....\r\n";
     }
     else
     {
         cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
     }
 return SMS(cmd);
}

typedef struct PP_
{
  UINT A;
  UINT B;
}PP;

void RandomPort(PP *port)
{
  DWORD r = GetTickCount();
  port->A = 128 + (r % 64);
  port->B = r % 0xff;
}

void GetClientIP(int sock, UINT *ip)
{
  socklen_t addr_size = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  getsockname(sock, (struct sockaddr *)&addr, &addr_size);
 
  UCHAR* host = inet_ntoa(addr.sin_addr);
  sscanf(host,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
}

int f_PASV(CLIENT *cmd,TSOCKET *S)
{
  if(cmd->logged_in)
  {
    UINT ip[4];
    PP port;
    UCHAR buff[255];
    int ret; 
    RandomPort(&port);
    GetClientIP(cmd->connection,ip);
    net_closesocket(&cmd->sock_pasv);

    ret = net_bind( &cmd->sock_pasv, (256*port.A)+port.B ,1);
    if( ret < 1)
    {   
        cmd->msg = "421 Impossibile creare il socket, nessun indirizzo IP utilizzabile trovato.\r\n";
    }
    else
    {   
        sprintf(buff,"227 Accesso alla modalità passiva (%d,%d,%d,%d,%d,%d)\r\n\0",
              ip[0],ip[1],ip[2],ip[3],port.A,port.B);
        cmd->msg = buff;
        cmd->mode = SERVER;
        cmd->offset = 0;
    }
  }
  else
  {
    cmd->msg = "530 Effettua il login con USER e PASS\r\n";
  }
  return SMS(cmd);
}

int f_PASS(CLIENT *cmd,TSOCKET *S)
{
     cmd->msg = "500 USER e PASS sono errati.\r\n";
     
     if(cmd->username_ok==1)
     {
          if(strcmp(cmd->arg,cmd->password)==0)
      {                  
              cmd->logged_in = 1;
              cmd->msg = "230 Login effettuato.\r\n";
          }
     }
 return SMS(cmd);
}

int f_QUIT(CLIENT *cmd,TSOCKET *S)
{
  cmd->msg = "221 Arrivederci.Non avrei mai pensato che sarebbe finita così.\r\n";
  SMS(cmd);
  return -1;
}

int f_SYST(CLIENT *cmd,TSOCKET *S)
{
     if(cmd->logged_in)
     {
         cmd->msg = "215 UNIX Type: L8\r\n";
     }
     else
     {
         cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
     }
  return SMS(cmd);
}

int f_TYPE(CLIENT *cmd,TSOCKET *S)
{
     if(!cmd->logged_in)
     {
         cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
     }
     else if(cmd->arg[0]=='I')
     {
         cmd->msg = "200 Commutazione alla modalità binaria.\r\n";
     }
     else if(cmd->arg[0]=='A')
     {
         cmd->msg = "200 Commutazione alla modalità ASCII.\r\n";
     }
     else
     {
         cmd->msg = "504 Comando non implementato per quel parametro.\r\n";
     }
 return SMS(cmd);
}

int f_USER(CLIENT *cmd,TSOCKET *S)
{
if(SendMessage(hAuth,BM_GETCHECK,0,0)==BST_CHECKED)
    {
        cmd->username_ok = 1;
        cmd->logged_in = 1;
        cmd->msg = "230 Login effettuato.\r\n";  
    } 
    else if(strcmp(cmd->arg,cmd->username)==0)
{
cmd->username_ok = 1;
cmd->msg = "331 USER ok, serve PASS.\r\n";
}
else
{
        cmd->msg = "530 USER errato.\r\n";
    }
 return SMS(cmd);
}

int f_NOOP(CLIENT *cmd,TSOCKET *S)
{
    if(cmd->logged_in)
{
cmd->msg = "200 Nice to NOOP you!\r\n";
}
else
{
cmd->msg = "530 NOOB hehe.\r\n";
}
return SMS(cmd);
}

  
