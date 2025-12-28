CFLAGS = -std=c2x
EXT_DEP= -lpcap -lsqlite3
SERVER = \
    main.c \
    $(wildcard lib/*.c) \
    $(wildcard lib/command/*.c) \
    $(wildcard lib/utils/*.c) \
    $(wildcard lib/ipc/socket/server/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \
    $(wildcard lib/utils/string/*.c) \
	
CLIENT = \
    lib/command/cmd_serializer.c \
    lib/command/cmd_builder.c \
    $(wildcard lib/utils/string/*.c) \
    $(wildcard lib/ipc/socket/client/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \
    $(wildcard lib/client/*.c) \

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
