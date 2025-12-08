CFLAGS = -std=c2x
EXT_DEP= -lpcap -lsqlite3
SERVER = \
    main.c \
    $(wildcard lib/*.c) \
    $(wildcard lib/command/*.c) \
    $(wildcard lib/utils/*.c) \
    $(wildcard lib/utils/string/*.c) \
    uds_common.c
	
CLIENT = \
    client.c \
    lib/command/cmd_serializer.c \
    $(wildcard lib/utils/string/*.c) \
    uds_common.c


main:$(SERVER)
	clang $(CFLAGS) $(SERVER) $(EXT_DEP) -o main
	
	
client:$(CLIENT)
	clang $(CFLAGS) $(CLIENT) -o client

run:main
	./main
	
run_client:client
	./client

