#include "main.h"

const char szClassName[ ] = "Windows_FTP_Server";

WSADATA wsaData;
void server();
void CenterOnScreen(HWND hnd);

void LoadInitStuff()
{
    Sleep(500); //let the GUI load
   	LPHOSTENT hostA; 
    struct in_addr myaddrA;	 
    UCHAR MyIP[250];
    ZeroMemory(MyIP,250);
    if(gethostname(MyIP,250)!=SOCKET_ERROR)
    {
	     if((hostA=gethostbyname(MyIP)) != NULL)
	     {
	         myaddrA= *(struct in_addr far *)(hostA->h_addr);
	         sprintf(MyIP," %s : 21\0",inet_ntoa(myaddrA));
             SetWindowText(hWnd,MyIP);
         }
	 }
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)  
   {
       case WM_CREATE:
       {  
            HFONT hFont;
            HWND f;
            hWnd = hwnd;
            hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Comic Sans MS");
            f=CreateWindow("BUTTON","USERNAME",WSV|BS_GROUPBOX|BS_CENTER,4,4,147,47,hwnd,NULL,ins,NULL);
            SETF(f);
            hUsername = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","anonymous",WSV|ES_AUTOHSCROLL, 14,24,128,22,hwnd,(HMENU)0,ins,NULL);
            SendMessage(hUsername, EM_LIMITTEXT,(WPARAM)100,(LPARAM)0);
            SETF(hUsername);
            f=CreateWindow("BUTTON","PASSWORD",WSV|BS_GROUPBOX|BS_CENTER,5,54,147,47,hwnd,NULL,ins,NULL);
            SETF(f);
            hPassword = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","anonymous",WSV|ES_AUTOHSCROLL,14,74,128,22,hwnd,NULL,ins,NULL);
            SendMessage(hPassword, EM_LIMITTEXT,(WPARAM)100,(LPARAM)0);
            SETF(hPassword); 
            hAuth=CreateWindow("BUTTON","Disable Login",WSV|BS_AUTOCHECKBOX|BS_TEXT,160,8,90,19,hwnd,NULL,ins,NULL);
            SETF(hAuth);
            hDelete=CreateWindow("BUTTON","ReadOnly",WSV|BS_AUTOCHECKBOX|BS_TEXT,160,31,90,19,hwnd,NULL,ins,NULL);
            SETF(hDelete);
            hStart=CreateWindow("BUTTON","Start",WSV,154,54,95,22,hwnd,(HMENU)ID_START,ins,NULL);
            SETF(hStart);             
            hStop=CreateWindow("BUTTON","Stop",WSV|WS_DISABLED,154,78,95,22,hwnd,(HMENU)ID_STOP,ins,NULL);
            SETF(hStop);             
            
            CenterOnScreen(hwnd);
            if(WSAStartup( MAKEWORD(2,0), &wsaData ) == SOCKET_ERROR )
            {
               SetWindowText(hwnd,"#error WSAStartup"); 
               EnableWindow(hStart,0);   
               EnableWindow(hUsername,0);
               EnableWindow(hPassword,0);
               break;
           }
           CreateThread(0,0,(LPTHREAD_START_ROUTINE)LoadInitStuff,0,0,0);
       }
       break;
       case WM_COMMAND:
       {
           switch(LOWORD(wParam))
           {           
               case ID_START:
               { 
                    EnableWindow(hStart,0);
                    EnableWindow(hUsername,0);
                    EnableWindow(hPassword,0);
                    CreateThread(0,0,(LPTHREAD_START_ROUTINE)server,0,0,0);
               } break;
               case ID_STOP:
               { 
                    EnableWindow(hStop,0);
                    net_closesocket(&Mainsock); 

               } break;      
           }//switch                                         
       }//WM_COMMAND
       break; 
     case WM_DESTROY:
     {
           WSACleanup();
           PostQuitMessage (0);
     }
     break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


int WINAPI WinMain(HINSTANCE _ins_,HINSTANCE _hp_,LPSTR _ar_,int nF)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    ins = _ins_;
    wincl.hInstance = _ins_;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hIconSm = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName  = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND +1;

    if(!RegisterClassEx (&wincl)) return 0;

    hwnd = CreateWindowEx(0,szClassName,"Server Ftp",WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,
         258,135,HWND_DESKTOP,NULL,_ins_,NULL);

    ShowWindow (hwnd, nF);
    
    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return messages.wParam;
}

void CenterOnScreen(HWND hnd)
{
  RECT rcClient, rcDesktop;
  int nX,nY;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
  GetWindowRect(hnd, &rcClient);
  nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
  nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
  SetWindowPos(hnd, NULL, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
return;
}


