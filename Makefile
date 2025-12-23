CFLAGS = -std=c2x
EXT_DEP= -lpcap -lsqlite3
SERVER = \
    main.c \
    $(wildcard lib/*.c) \
    $(wildcard lib/command/*.c) \
    $(wildcard lib/utils/*.c) \
    $(wildcard lib/server/*.c) \
    $(wildcard lib/utils/string/*.c) \
    protocol.c
	
CLIENT = \
    client.c \
    lib/command/cmd_serializer.c \
    lib/request_handler.c \
    $(wildcard lib/utils/string/*.c) \
    protocol.c

run:main
	./main
	
run_client:client
	./client

main:$(SERVER)
	clang $(CFLAGS) $(SERVER) $(EXT_DEP) -o main
	
client:$(CLIENT)
	clang $(CFLAGS) $(CLIENT) -o client

# server debug
debug_main:main_d
	lldb ./main

main_d:$(SERVER)
	clang -g -O0 $(CFLAGS) $(SERVER) $(EXT_DEP) -o main
    
# client debug
debug_client:client_d
	lldb ./client

client_d:$(CLIENT)
	clang -g -O0 $(CFLAGS) $(CLIENT) -o client
