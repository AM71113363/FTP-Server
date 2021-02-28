#include "main.h"

UCHAR welcome[] = "220 Welcome to nice FTP service.\r\n";

UINT FindCommandInList(UCHAR *cmd)
{
    UINT i = 0;
    while(CMDCOMMANDS[i].cmd!=NULL)
    {
        if(strcmp(cmd,CMDCOMMANDS[i].cmd)==0)
           return i;
      i++;
    }
  return 0xFFFF;
}

int response(CLIENT *cmd,TSOCKET *S)
{
   UINT i = FindCommandInList(cmd->command);
   if(i!=0xFFFF)
   {
     return CMDCOMMANDS[i].func(cmd,S); 
   }
   else
   { 
      cmd->msg = "500 Comando sconosciuto\r\n";
      return SMS(cmd);
  }
}


void parse_command(CLIENT *cmd, UCHAR *buf)
{ 
    UINT len,i,n;
    UCHAR *p;  
    UCHAR Temp[MAX_PATH];
     
    memset(Temp,0,MAX_PATH);
    memset(cmd->command,0,5);
    memset(cmd->arg,0,MAX_PATH);
     
    len = (UINT)strlen(buf);
    for(i=0;i<len;i++)
    {
          if(buf[i] == '\r')          
               break;           
          else if(buf[i] == '\n')
               break;
          else
               Temp[i] = buf[i];
    }
    Temp[i] = 0;
    len = (UINT)strlen(Temp);
    for(i=0;i<len;i++)
    {
        if(Temp[i] == ' ')
          break;            
    }
    if((i < 3 ) || (i > 4 ) )
    {
        cmd->command[0] = 0;    
        cmd->arg[0] = 0;   
        return;    
    }
    snprintf(cmd->command,i,"%s\0",Temp);
    for(n=0;n<i;n++)
    {
        cmd->command[n] = toupper(cmd->command[n]);
    }
     
    if(i == len)
    { 
        cmd->arg[0] = 0;    
          return;
    }
    p = &Temp[i];
    while(p[0] == ' ')
       p++;
    if(p[0] == 0)
    {
          cmd->arg[0] = 0; 
          return;
    }  
    if(p[0] == '-')
    {
        while( (p[0] != ' ') && (p[0] != 0) )
             p++;   
        while(p[0] == ' ')
              p++;
        if(p[0] == 0)
        {
            cmd->arg[0] = 0; 
               return;
        }
        else
        {
            sprintf(cmd->arg,"%s\0",p); 
            return;
        }
     }
     else
     {
           sprintf(cmd->arg,"%s\0",p);  
           return;
     }
     cmd->arg[0] = 0;    
     return; 
}

void ResetClient(CLIENT *client)
{
     memset(client->command,0,5);
     memset(client->arg,0,MAX_PATH);
     memset(client->CurrDir,0,MAX_PATH);
     memset(client->Temp,0,MAX_PATH);
     memset(client->username,0,101);   
     memset(client->password,0,101);   
     client->offset = 0;  
     client->sock_pasv = -1;
}

void Worker(LPVOID _a_)
{
    TPASS *X = (TPASS*)_a_;
    int connection = X->c; 
    UCHAR buffer[BSIZE];
    int ret;
    TSOCKET S;
    CLIENT cmd;
    fd_set read_fds;
    ResetClient(&cmd);
    
    sprintf(cmd.username,"%s\0",X->username);   
    sprintf(cmd.password,"%s\0",X->password); 
    
    S.sock = X->c;
    
    ret = net_send( &connection, welcome, strlen(welcome));
    if( ret == ME_ERROR)
    {
        net_closesocket(&connection);
        return;
    }
    memset(buffer,0,BSIZE);
    ret = net_isrecv(connection,&read_fds,30);
    if(ret != 1)
    {
        net_closesocket(&connection);
        return;
    }       
    ret = recv( connection, buffer, BSIZE-1,0);
    if( ret < 1)
    {
        net_closesocket(&connection);
        return;
    }
    memset(cmd.CurrDir,0,MAX_PATH);  
    cmd.CurrDir[0] = '\\';  
    
    while(ret)
    {
        buffer[ret] = 0;
        parse_command(&cmd, buffer);
        cmd.connection = connection;
        ret = response(&cmd,&S);
        if( ret == ME_ERROR)
             break;
        memset(buffer,0,BSIZE);
        memset(cmd.command,0,5);
        memset(cmd.arg,0,MAX_PATH);
        ret = net_isrecv(connection,&read_fds,30); //-1=error,0=timeout,1=ok
        if(ret != 1)
           break;
        ret = recv( connection, buffer, BSIZE-1,0);
        if( ret < 1)
           break;
     }//while ret 
     net_closesocket(&connection);     
}


void server()
{
    int Client;
    int ret;
    UCHAR username[101]="anonymous";
    UCHAR password[101]="anonymous"; 
    
    ret = net_bind( &Mainsock, 21 , 4);
    if(ret == ME_ERROR)
    {
          SetWindowText(hWnd,"#Error Bind");
          EnableWindow(hStart,1);
          EnableWindow(hUsername,1);
          EnableWindow(hPassword,1); 
          return;
    }
    //get Settings 
    if(GetWindowTextLength(hUsername))
    {
          memset(username,0,101);                            
          if(!GetWindowText(hUsername,username,100))
             sprintf(username,"%s\0","anonymous");
    }
    SetWindowText(hUsername,username);                                   
    if(GetWindowTextLength(hPassword))
    {
          memset(password,0,101);                            
          if(!GetWindowText(hPassword,password,100))
             sprintf(password,"%s\0","anonymous");
    }
    SetWindowText(hPassword,password); 
    EnableWindow(hStop,1);
    while(1)
    {
          ret = net_accept( Mainsock, &Client);
          if( ret == ME_ERROR)
          { 
               break; 
          }
          else
          {
               TPASS *X;
               X = (TPASS*)malloc(sizeof(TPASS));
               memset(X, 0, sizeof(TPASS));
               X->c = Client;
               memset(X->username,0,101);   
               memset(X->password,0,101);  
               sprintf(X->username,"%s\0",username);   
               sprintf(X->password,"%s\0",password);   
              
               CreateThread(0,0,(LPTHREAD_START_ROUTINE)Worker,(LPVOID)X,0,0);
               _sleep(2000);
               free(X);
          } 
            
     }//infinite loop    
     net_closesocket(&Mainsock); 
     EnableWindow(hStop,0);
     EnableWindow(hStart,1);
     EnableWindow(hUsername,1);
     EnableWindow(hPassword,1); 
}

