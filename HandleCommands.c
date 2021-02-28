#include "main.h"

#define LBOX ((DBOX*)lParam) 

DWORD up = 20;

typedef struct _dbox
{
    HWND hMain;
    HWND hStat;
    DWORD offset;
    DWORD length;
    UCHAR PgBar[128];
}DBOX;


void CenterOn(HWND hnd)
{
     RECT rcClient, rcDesktop;
     SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
     GetWindowRect(hnd, &rcClient);
     int nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
     int nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
     if(up > nY) up = 20;
     SetWindowPos(hnd, NULL, nX, nY-up, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
 SetWindowPos(hnd,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);                   
}

BOOL CALLBACK DlgProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
       case WM_INITDIALOG:
       {
            up += 47;
            CenterOn(hnd);
            LBOX->hMain = hnd;
            LBOX->hStat = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE ,11,11, 274, 20, hnd,0, ins, NULL);                
            SetWindowLong(hnd,GWL_USERDATA,(LONG)lParam);
            SetTimer(hnd,1,200,NULL);  
            SetWindowText(hnd,LBOX->PgBar);     
       }
       break;
       case WM_TIMER:
       {
            DBOX *A = (DBOX*)GetWindowLong(hnd,GWL_USERDATA);
            if(A->length > 0)
            {
                sprintf(A->PgBar,"[ %u / %u ]\0",A->offset,A->length);
            }
            else
            {
                sprintf(A->PgBar,"[ %u ]\0",A->offset);
            }            
            SetWindowText(A->hStat,A->PgBar);
            
       }
       break;
       case WM_CLOSE:
       case WM_DESTROY:
       {    
           KillTimer( hnd,1);
           EndDialog(hnd,1);
       }
       break;
    }
    return FALSE;
}


void Go(LPVOID _a_)
{
    DialogBoxParam(ins,MAKEINTRESOURCE(2222),0,DlgProc,(LPARAM)_a_);
}

UCHAR *make_path(CLIENT *cmd)
{
    UINT i,len,n;
     
    memset(cmd->Temp,0,MAX_PATH);
     
    if(cmd->arg[0] == 0)
    {
        sprintf(cmd->Temp,"%s\0",cmd->CurrDir);
    }
    else if((cmd->arg[0] == '\\') || (cmd->arg[0] == '/') ) // root dir
    {      
        if(cmd->arg[1] == 0)
        {
            sprintf(cmd->Temp,"%s\0","\\");                          
        }
        else
        {  
            len = (UINT)strlen(cmd->arg);
            for(i=0;i<len;i++)
            {  
                if(cmd->arg[i] == '/')
                    cmd->Temp[i] = '\\';
                  else
                    cmd->Temp[i] = cmd->arg[i];                
            } 
        } 
    }
    else
    {
        if(cmd->CurrDir[1] == 0)
        {
              n = sprintf(cmd->Temp,"%s",cmd->CurrDir);
        }
        else
        {
              n = sprintf(cmd->Temp,"%s\\",cmd->CurrDir);
        }
        len = (UINT)strlen(cmd->arg);
        for(i=0;i<len;i++)
        {  
            if(cmd->arg[i] == '/')
                cmd->Temp[i+n] = '\\';
            else
                cmd->Temp[i+n] = cmd->arg[i];                
        } 
    }
    len = (UINT)strlen(cmd->Temp);
    if((cmd->Temp[len-1] == '\\') && (len > 1) )
    {
        cmd->Temp[len-1] = 0;          
    }
return cmd->Temp;
}

int FileExist(UCHAR *DirOrFile)
{
    DWORD st= GetFileAttributes(DirOrFile);
    
if(st>=0)
    { 
       if((st & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)       
       {
            return ThisIsDir;
       }
       else
       {
            return ThisIsFile;
       }
    }
    return DontExist;
}


int f_CDUP(CLIENT *cmd,TSOCKET *S)
{
    UCHAR *p;
     
    if(!cmd->logged_in)
    {
          cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
          return SMS(cmd);
    }
    //virus client,want UP from root   
    if(cmd->CurrDir[1] == 0) // this is root dir cant go up
    {
          cmd->msg = "550 Impossibile cambiare la directory.\r\n";
          return SMS(cmd);  
    }
    p = strrchr(cmd->CurrDir,'\\');
    if(!p)//no slash, change to root dir
    {
          cmd->msg = "550 Impossibile cambiare la directory.\r\n";
          return SMS(cmd);  
    }
    if(p-cmd->CurrDir == 0) //virus client,want UP from root 
    {
          strcpy(cmd->CurrDir,"\\\0");  
    }
    else
    {
         p[0] = 0;//delete this slash
         sprintf(cmd->CurrDir,"%s\0",cmd->CurrDir);
    }
    p=cmd->CurrDir;
    p++;
    if(p[0] != 0)
    {
        if(!FileExist(p))
        {
          cmd->msg = "550 Impossibile cambiare la directory.\r\n";
          return SMS(cmd);  
        }
    }
    return f_PWD(cmd,S);    
}


int f_CWD(CLIENT *cmd,TSOCKET *S)
{
    UCHAR *p;
    UINT i,len;
    UCHAR ThisDir[MAX_PATH];
    UCHAR msg[MAX_PATH];

    if(!cmd->logged_in)
    {
        cmd->msg = "500 Effettua il login con USER e PASS.\r\n";
        return SMS(cmd); 
    }                  
    p = make_path(cmd);
     
    sprintf(cmd->CurrDir,"%s\0",p);
    memset(msg,0,MAX_PATH);
    len = (UINT)strlen(p);
    
    for(i=0;i<len;i++)
    {
          if(p[i] == '\\')
          {
               msg[i] = '/';
          }
          else
          {
              msg[i] = p[i]; 
          }
    } 
    p=cmd->CurrDir;
    p++;
    if(p[0] != 0)
    {
        if(!FileExist(p))
        {
          cmd->msg = "550 Impossibile cambiare la directory.\r\n";
          return SMS(cmd);   
        }
    }
    sprintf(ThisDir,"250 Comando riuscito. \"%s\" è la directory corrente.\r\n\0",msg);
    cmd->msg = ThisDir;
    return SMS(cmd);
}

int f_DELE(CLIENT *cmd,TSOCKET *S)
{
    UCHAR ThisDir[MAX_PATH];
    UINT i,len; 
    UCHAR *p;
    memset(ThisDir,0,MAX_PATH);      

    if(!cmd->logged_in)
    {
          cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
          return SMS(cmd);
    }
    else if(SendMessage(hDelete,BM_GETCHECK,0,0)==BST_CHECKED)
    {
          cmd->msg = "550 Permesso negato\r\n";
          return SMS(cmd); 
    }    
    p = make_path(cmd);
    p++;
    if(p[0] == 0)//this maybe Client bug
    {
         cmd->msg = "550 Nome file non valido.\r\n";           
    }             
    else if(DeleteFile(p)==0)
    {
         cmd->msg = "500 Impossibile eliminare il file.\r\n"; //it was 550
    }
    else
    {
         cmd->msg = "250 Azione richiesta per il file, ok, completata.\r\n";
    }
 return SMS(cmd);
}


UCHAR sdays[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
UCHAR smonth[][4] = {"NULL","Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//dir Can't contain first slash,but for sub dir must have last slash
int SendDir(int connection,UCHAR *Dir)
{
    HANDLE h;
    WIN32_FIND_DATA info;
    BOOL IsDir;
    DWORD size;
    UCHAR current_dir[BSIZE];

    UINT len;
    int ret = 1;
     
    GetCurrentDirectory(MAX_PATH,current_dir);
    memset(current_dir,0,BSIZE);
    h = FindFirstFile(Dir, &info);
    if(h == INVALID_HANDLE_VALUE)
    { 
         return -1;
    }
    do
    {
         if(!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
         {
               if((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
           {
                    IsDir = TRUE;
               }
               else
               {
                    IsDir = FALSE;
               }
               memset(current_dir,0,BSIZE);
                            
               len = sprintf(current_dir,
                                  "%crwxrwxrwx  1 user group %16u %s %s\r\n\0", 
                                  (IsDir==TRUE )?'d':'-',
                                  info.nFileSizeLow,
                                  "Sep 01 22:22",
                                  info.cFileName);
               ret = net_send( &connection, current_dir,len);
               if(ret < 1 )
                    break; 
          }
    } while(FindNextFile(h, &info) != 0);
    FindClose(h);
    return ret;
}


int f_LIST(CLIENT *cmd,TSOCKET *S)
{
     int connection;
     UCHAR CurrDir[MAX_PATH];
     UINT i,len,ret;
     UCHAR *p;     
     
     if(!cmd->logged_in)
     {    
          cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
          cmd->mode = NORMAL;
          return SMS(cmd);
     }
     if(cmd->mode != SERVER)
     {
          cmd->msg = "425 Utilizzare prima PASV o PORT.\r\n";
          cmd->mode = NORMAL;
          return SMS(cmd);
     }  
     //check if the client first call PASV
     if(cmd->sock_pasv < 1)
     {
          cmd->msg = "503 Sequenza di comandi Errata.\r\n";
          cmd->mode = NORMAL;
          return SMS(cmd);
     }      
     
     memset(CurrDir,0,MAX_PATH);
    
     p = make_path(cmd);

     p++;
     if(p[0] == 0)
     {
           sprintf(CurrDir,"%s\0","*"); 
     }
     else
     {
           sprintf(CurrDir,"%s\\*\0",p); 
     }
    
     
     ret = net_accept(cmd->sock_pasv, &connection);
     net_closesocket(&cmd->sock_pasv);
     if( ret < 1) //it shoudn't return -1
     {
          cmd->msg = "503 Sequenza di comandi Errata.\r\n";
          cmd->mode = NORMAL;
          return SMS(cmd);
     }
     cmd->msg = "150 Ecco la lista delle directory.\r\n";
     ret = SMS(cmd);
     if(ret < 1)
     {
         net_closesocket(&connection);  
         return ret;
     } 
     if(SendDir(connection,CurrDir) < 1)
     {
                           
     }                     
     cmd->msg = "226 Directory inviata OK.\r\n";
     cmd->mode = NORMAL;
     net_closesocket(&connection);
  return SMS(cmd);
}


int SendInfo(CLIENT *cmd,UCHAR *Dir)
{
    HANDLE h;
    WIN32_FIND_DATA info;
    UCHAR TimeBuf[MAX_PATH];
    h = FindFirstFile(Dir, &info);
    if(h == INVALID_HANDLE_VALUE)
    { 
         cmd->msg = "550 Could not get file ifo.\r\n";
    }
    else
    {
        FindClose(h);
        sprintf(TimeBuf,"213 20210901222222\r\n\0");
        cmd->msg =  TimeBuf;          
    }
 return SMS(cmd);
}

int f_MDTM(CLIENT *cmd,TSOCKET *S)
{
     UCHAR *p;
     UCHAR ThisDir[MAX_PATH];
        
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
           return SMS(cmd);
     }

     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {
           cmd->msg = "550 Impossibile ottenere informazioni sul file.\r\n";
           return SMS(cmd);
     }
     else
     {  
        if(!FileExist(p))
        {
          cmd->msg = "550 Impossibile ottenere informazioni sul file.\r\n";
          return SMS(cmd); 
        }
        return SendInfo(cmd,p);  
     } 
}


int f_MKD(CLIENT *cmd,TSOCKET *S)
{
     UCHAR ThisDir[MAX_PATH];
     UCHAR res[MAX_PATH];
     UINT i,len;
     UCHAR *p;     
     
     if(!cmd->logged_in)
     {
          cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
          return SMS(cmd);
     }
      
     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {
         cmd->msg = "550 Impossibile creare la directory. Controlla il percorso o le autorizzazioni.\r\n";
     }                       
     else if(CreateDirectory(p,NULL) != 0)
     {
          len = (UINT)strlen(p);
          p--;
          for(i=0;i<len;i++)
          {
              if(p[i] == '\\')
              {
                   res[i] = '/';
              }
              else
              {
                  res[i] = p[i];
              }                         
          }                      
          sprintf(ThisDir,"257 \"%s\" nuova directory creata.\r\n\0",res);
          cmd->msg = ThisDir;
     }
     else
     {
          cmd->msg = "550 Impossibile creare la directory. Controlla il percorso o le autorizzazioni.\r\n";
     }
  return SMS(cmd);
}


int SendMLST(CLIENT *cmd, UCHAR *Dir)
{
    HANDLE h;
    WIN32_FIND_DATA info;
    UCHAR ThisDir[MAX_PATH];
    UCHAR TimeBuf[MAX_PATH];
    UCHAR AllData[1024];
    UINT i,len;
    int ret;
    UCHAR *type = "X";
    h = FindFirstFile(Dir, &info);
    if(h == INVALID_HANDLE_VALUE)
    { 
         cmd->msg = "550 File or directory not found.\r\n";
    }
    else
    {
        FindClose(h);
        if((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
        {
            type = "dir";
        }
        else
        {
            type = "file";
        }
        memset(TimeBuf,0,MAX_PATH);
        memset(ThisDir,0,MAX_PATH);
        memset(AllData,0,1024);
        
        len = (UINT)strlen(Dir);
        
        for(i=0;i<len;i++)
        {
             if(Dir[i] == '\\')
                 ThisDir[i] = '/';  
             else
                 ThisDir[i] = Dir[i];          
        }

       sprintf(AllData," Type=%s;Size=%u;Modify=20210901222222; /%s\r\n\0",
                         type,info.nFileSizeLow,ThisDir);
       
       sprintf(TimeBuf,"250-Listing %s\r\n\0",cmd->arg);
       cmd->msg =  TimeBuf; 
       ret = SMS(cmd);
       if(ret == ME_ERROR)
          return ret;
       cmd->msg =  AllData; 
       ret = SMS(cmd);
       if(ret == ME_ERROR)
          return ret;
       cmd->msg =  "250 End\r\n";          
    }

    return SMS(cmd);
}


int f_MLST(CLIENT *cmd,TSOCKET *S)
{
     UCHAR *p;
     UCHAR ThisDir[MAX_PATH];     
     
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
           return SMS(cmd);
     }

     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {
           cmd->msg = "550 Nome file non valido.\r\n";
           return SMS(cmd);
     }
     else
     {  
        return SendMLST(cmd,p);
     } 
}


int f_PWD(CLIENT *cmd,TSOCKET *S)
{
     UCHAR ThisDir[MAX_PATH];
     UCHAR tmp[1024];
     UINT i,len;     
     
     if(!cmd->logged_in)
     {
         cmd->msg = "550 Impossibile ottenere PASS.\r\n";
         return SMS(cmd);
     }             

     memset(ThisDir, 0, MAX_PATH);
     memset(tmp, 0, 1024);
     
     ThisDir[0]='/';
     
     len = (UINT)strlen(cmd->CurrDir);
              
     for(i = 0; i < len; i++)
     {
        if(cmd->CurrDir[i]=='\\') 
            ThisDir[i]='/';
        else
            ThisDir[i] = cmd->CurrDir[i];      
     }                      
     sprintf(tmp,"257 \"%s\" è la directory corrente.\r\n\0",ThisDir);
     cmd->msg = tmp;
     return SMS(cmd);
}


int f_REST(CLIENT *cmd,TSOCKET *S)
{
    UCHAR msg[200]; 
    if(!cmd->logged_in)
    {
        cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
        cmd->mode = NORMAL;
    }
    else if(cmd->mode != SERVER)
    {
        cmd->msg = "550 Si prega di utilizzare PASV invece di PORT.\r\n";
    }
    //check if the client first call PASV
    else if(cmd->sock_pasv < 1)
    {
          cmd->msg = "503 Ssequenza di comandi Errata.\r\n";
    } 
    else
    {
        DWORD offset;
        sscanf(cmd->arg,"%u",&offset);
        cmd->offset = offset;
        sprintf(msg,"350 REST supportato. Pronto a riprendere all byte offset %u.\r\n\0",offset);
        cmd->msg = msg;
    }
    return SMS(cmd);
}


DWORD SendFile(int out_fd,HANDLE in_fd,DWORD offset, DWORD len, DBOX *D)
{
     DWORD count = len;

     DWORD orig;
     BOOL rRead;
     UCHAR buf[4096];
     int numSent,ret;
     DWORD numRead, totSent;


     D->offset = offset;
     D->length = len; 
        
    if(offset > 0)
    {
        if (SetFilePointer(in_fd, offset, 0, FILE_BEGIN) == 0xFFFFFFFF)
        {
            return 0xFFFFFFFF;
        }
    }

    totSent = 0;
    while (count > 0)
    {
        rRead = ReadFile(in_fd,buf,4096, &numRead, NULL);
        if(rRead == FALSE)
        {
            return 0xFFFFFFFF;
        }
        else if(numRead == 0)
        {
             break;
        }
        ret =net_send( &out_fd, buf,numRead);
        if (ret < 1) 
        { 
            return 0xFFFFFFFF;
        }
        count -= numRead;
        totSent += numRead;
        D->offset = totSent;
    }
  return totSent;
}

int f_RETR(CLIENT *cmd,TSOCKET *SS)
{
    int connection;
    DWORD sent_total = 0;
    DWORD len;
    HANDLE fd;
    int ret;
    UCHAR *p;
    UINT i,ln;
    UCHAR ThisDir[MAX_PATH];
    DBOX D;   
    HANDLE mHandle;
    TSOCKET S;
     
    if(!cmd->logged_in)
    {
        cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
        cmd->mode = NORMAL;
        net_closesocket(&cmd->sock_pasv); 
        return SMS(cmd);
    }
    if(cmd->mode != SERVER)
    {
        cmd->msg = "550 Please use PASV instead of PORT.\r\n";
        cmd->mode = NORMAL;
        net_closesocket(&cmd->sock_pasv); 
        return SMS(cmd);
    }
    
    p = make_path(cmd);
    p++;
    fd = CreateFile(p, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if(fd == INVALID_HANDLE_VALUE)
    {
         cmd->msg = "550 Failed to get file\r\n";
        cmd->mode = NORMAL;
        net_closesocket(&cmd->sock_pasv); 
        return SMS(cmd);
    }
          
    cmd->msg = "150 Opening BINARY mode data connection.\r\n";
    ret = SMS(cmd);
    if(ret < 1)
        return ret;
    ret = net_accept( cmd->sock_pasv, &connection);  
    net_closesocket(&cmd->sock_pasv); 
    if(connection < 1) // it shouldn't happen
    {
        cmd->msg = "550 Failed to read file.\r\n";
        cmd->mode = NORMAL;
        return SMS(cmd);
    }
    D.hMain = NULL;
    D.hStat = NULL;
    D.offset = 0;
    D.length = 0;
    
    mHandle = CreateThread(0,0,(LPTHREAD_START_ROUTINE)Go,(LPVOID)&D,0,0);
    CloseHandle(mHandle);
    while(D.hStat == NULL)
       _sleep(247);
    
    if(cmd->offset > 0)
        SetWindowText( D.hMain,"Rest Upload");
    else
        SetWindowText( D.hMain,"Upload");
    
    len = GetFileSize(fd,NULL);
    
    S.sock = connection;
    sent_total = SendFile(connection,fd,cmd->offset,len,&D);

    EndDialog(D.hMain,1);
                   
    if(sent_total == 0xFFFFFFFF)
    {
        cmd->msg = "550 Failed to read file.\r\n";  
    }        
    else if((sent_total +cmd->offset) != len)
    {
        cmd->msg = "550 Failed to read file.\r\n";
    }
    else
    {
        cmd->msg = "226 File inviato OK.\r\n";
    }

    CloseHandle(fd);
    net_closesocket(&connection);
    cmd->mode = NORMAL;
  return SMS(cmd);
}


int f_RMD(CLIENT *cmd,TSOCKET *S)
{
     UCHAR ThisDir[MAX_PATH];
     UINT i,len;
     UCHAR *p;     

     if(!cmd->logged_in)
     {
           cmd->msg = "530 Per favore fai prima il Login.\r\n";
           return SMS(cmd);
     }

     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {
         cmd->msg = "550 Impossibile eliminare la directory.\r\n";
     }
     else if(RemoveDirectory(p)!=0)
     {
         cmd->msg = "250 Azione richiesta per il file, ok, completata.\r\n";
     }
     else
     {
         cmd->msg = "550 Impossibile eliminare la directory.\r\n";
     }
  return SMS(cmd);
}


int f_RNFR(CLIENT *cmd,TSOCKET *S)
{
     UCHAR *p; 
     
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
           return SMS(cmd);
     }
     
     p = make_path(cmd);
     p++;
     
     if(!FileExist(p))
     {
         cmd->msg = "550 Nome file non valido.\r\n";
     }
     else
     {
         cmd->msg = "350 Il file esiste, pronto per il nome della destinazione.\r\n";
     }
  return SMS(cmd);
}


int f_RNTO(CLIENT *cmd,TSOCKET *S)
{
     UCHAR *p;
     UCHAR ThisDir[MAX_PATH];     
     
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
           return SMS(cmd);
     }

     memset(ThisDir,0,MAX_PATH);
     //copy previous dir filename
     p = cmd->Temp;
     p++;
     sprintf(ThisDir,"%s\0",p);
     p = make_path(cmd);
     p++;
     //just check
     if(p[0] == 0)
     {
         cmd->msg = "503 Sequenza di comandi Errata!\r\n";
         sprintf(cmd->Temp,"\\%s\0",ThisDir);  //reset cuz make_path(cmd); made a new path
     }
     else
     {
          if(MoveFile(ThisDir,p) == 0)
          {
              cmd->msg = "550 Permesso negato\r\n"; 
              sprintf(cmd->Temp,"\\%s\0",ThisDir);  //reset cuz make_path(cmd); made a new path                  
          }
          else
          {
              cmd->msg = "250 file rinominato correttamente\r\n";
          }
     }
  return SMS(cmd);
}


int f_SIZE(CLIENT *cmd,TSOCKET *S)
{
     HANDLE h;
     WIN32_FIND_DATA info;

     UCHAR filesize[128];
     UCHAR *p;     
     
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Effettua il login con USER e PASS.\r\n";
           return SMS(cmd);
     }
 
     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {
           cmd->msg = "550 Impossibile ottenere la dimensione del file.\r\n";
     }
     else
     {
          h = FindFirstFile(p, &info);
        if(h == INVALID_HANDLE_VALUE)
            { 
                cmd->msg = "550 Impossibile ottenere la dimensione del file.\r\n";
            }
            else
            {
                FindClose(h);
                memset(filesize,0,128);           
                sprintf(filesize, "213 %u\r\n\0", info.nFileSizeLow);
                cmd->msg = filesize;
           }
     }//else
  return SMS(cmd);
}


int RecvFile(int connection,HANDLE fd,DWORD offset,DBOX *D)
{
     int ret;
     DWORD doffset;
     fd_set read_fds;
     UCHAR buffer[4096];
     UCHAR status[100];
     DWORD len = offset;
     
     D->offset = offset;
     
     if(offset > 0)
     {
        if (SetFilePointer(fd, offset, 0, FILE_BEGIN) == 0xFFFFFFFF)
        {
            return -1;
        }
     }
     
     while(1)
     {
         ret = net_isrecv(connection,&read_fds,30); //-1=error,0=timeout,1=ok
         if(ret != 1)
             break;
         
         ret = recv( connection, buffer, 4096,0);
         if(ret < 1)
         {
             break;
         }
         else
         {
            len += ret; 
            D->offset = len;
            sprintf(status,"[ %u ]\0",len);
            SetWindowText(D->hStat,status);
             
         }
         if(WriteFile(fd,buffer,ret,&doffset,NULL) == FALSE)
            return -1;
     } 
  return 1;
}

int f_STOR(CLIENT *cmd,TSOCKET *SS)
{
     int connection;
     HANDLE fd;
     DBOX D;
     UCHAR *p;  
     int ret;
     TSOCKET S;
     
     if(!cmd->logged_in)
     {
           cmd->msg = "530 Per favore fai prima il Login.\r\n";
           cmd->mode = NORMAL;
           net_closesocket(&cmd->sock_pasv);
           return SMS(cmd);
     }
     if(cmd->mode != SERVER)      
     {
           cmd->msg = "550 Si prega di utilizzare PASV invece di PORT.\r\n";
           cmd->mode = NORMAL;
           net_closesocket(&cmd->sock_pasv);
           return SMS(cmd);
     }   

     p = make_path(cmd);
     p++;
     if(p[0] == 0)
     {      
           cmd->msg = "550 Nome file non valido.\r\n";  
           cmd->mode = NORMAL;
           net_closesocket(&cmd->sock_pasv);
           return SMS(cmd);
     }
     
     ret = net_accept( cmd->sock_pasv, &connection);
     net_closesocket(&cmd->sock_pasv);
     if( ret < 1) //it shouldn't happen
     {
           cmd->msg = "550 Permesso negato\r\n";
           cmd->mode = NORMAL;
           return SMS(cmd);
     }
     S.sock = connection;

     cmd->msg = "125 Connessione dati già aperta; inizio del trasferimento.\r\n";
     ret = SMS(cmd);
     if(ret < 1)
        return ret;
     //create file  or append it
     fd = CreateFile(p, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_ALWAYS, 0, 0);
     if(fd == INVALID_HANDLE_VALUE)
     {
           cmd->msg = "550 Permesso negato\r\n";
           cmd->mode = NORMAL;
           net_closesocket(&connection);
           return SMS(cmd);
     }
     
     D.hMain = NULL;
     D.hStat = NULL;
     D.offset = 0;
     D.length = 0;
     
    if(cmd->offset > 0)
        strcpy( D.PgBar,"Rest Download\0");
    else
        strcpy( D.PgBar,"Download\0");
         
     CreateThread(0,0,(LPTHREAD_START_ROUTINE)Go,(LPVOID)&D,0,0);
     while(D.hStat == NULL)
       _sleep(100);
    

    ret = RecvFile(connection,fd,cmd->offset,&D);
    CloseHandle(fd); 
    EndDialog(D.hMain,1);
    net_closesocket(&connection);
    if(ret < 1)
    {
        cmd->msg = "550 Impossibile scrivere il file.\r\n";   
    }
    else
    { 
        cmd->msg = "226 File inviato OK.\r\n";
    }
     cmd->mode = NORMAL;
 return SMS(cmd);
}


