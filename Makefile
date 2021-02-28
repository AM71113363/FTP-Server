# Project: AM71113363



CC   = gcc.exe -s
WINDRES = windres.exe
RES  = main.res
OBJ  = main.o server.o net_socket.o JustReply.o HandleCommands.o $(RES)
LINKOBJ  = main.o server.o net_socket.o JustReply.o HandleCommands.o $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -mwindows -lws2_32  
INCS =  -I"C:/Dev-Cpp/include" 
BIN  = FTP-Server.exe
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before FTP-Server.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "FTP-Server.exe" $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

server.o: server.c
	$(CC) -c server.c -o server.o $(CFLAGS)

net_socket.o: net_socket.c
	$(CC) -c net_socket.c -o net_socket.o $(CFLAGS)

JustReply.o: JustReply.c
	$(CC) -c JustReply.c -o JustReply.o $(CFLAGS)

HandleCommands.o: HandleCommands.c
	$(CC) -c HandleCommands.c -o HandleCommands.o $(CFLAGS)

main.res: main.rc 
	$(WINDRES) -i main.rc --input-format=rc -o main.res -O coff 
