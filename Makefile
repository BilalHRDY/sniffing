# CFLAGS = -std=c2x
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

# run:main
# 	./main
	
# run_client:client
# 	./client

# main:$(SERVER)
# 	clang $(CFLAGS) $(SERVER) $(EXT_DEP) -o main
	
# client:$(CLIENT)
# 	clang $(CFLAGS) $(CLIENT) -o client

# # server debug
# debug_main:main_d
# 	lldb ./main

# main_d:$(SERVER)
# 	clang -g -O0 $(CFLAGS) $(SERVER) $(EXT_DEP) -o main
    
# # client debug
# debug_client:client_d
# 	lldb ./client

# client_d:$(CLIENT)
# 	clang -g -O0 $(CFLAGS) $(CLIENT) -o client


###########  TEST ###########

ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=out
endif

.PHONY: clean
.PHONY: test

PATH_UNITY = unity/src/
PATH_LIB = lib/utils/calc/
PATH_TEST = test/
PATH_BUILD = build/
PATH_DEP = build/depends/
PATH_OBJ = build/objs/
PATH_RES = build/results/

# 
BUILD_PATHS = $(PATH_BUILD) $(PATH_DEP) $(PATH_OBJ) $(PATH_RES)

# test/Testcalc.c
SRCT = $(wildcard $(PATH_TEST)*.c)

COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS=-I$(PATH_UNITY)
# CFLAGS=-I. -I$(PATH_UNITY) -I$(PATH_LIB) -DTEST

# Pour test/Testcalc.c on a build/results/Testcalc.txt
RESULTS = $(patsubst $(PATH_TEST)Test%.c,$(PATH_RES)Test%.txt,$(SRCT) )

PASSED = `grep -s PASS $(PATH_RES)*.txt`
FAIL = `grep -s FAIL $(PATH_RES)*.txt`
IGNORE = `grep -s IGNORE $(PATH_RES)*.txt`

# test: build/ build/depends/ build/objs/ build/results/ build/results/Testcalc.txt
$(info $$mon log = $(RESULTS))
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# build/results/Testcalc.txt: build/Testcalc.out
$(PATH_RES)%.txt: $(PATH_BUILD)%.$(TARGET_EXTENSION)
# exécute Testcalc.out - redirige la sortie vers Testcalc.txt
	-./$< > $@ 2>&1

# build/Testcalc.out: build/objs/Testcalc.o build/objs/calc.o build/objs/unity.o
# Lier les fichiers objets pour créer l'exécutable.
$(PATH_BUILD)Test%.$(TARGET_EXTENSION): $(PATH_OBJ)Test%.o $(PATH_OBJ)%.o $(PATH_OBJ)unity.o
# gcc -o build/Testcalc.out build/objs/Testcalc.o build/objs/calc.o build/objs/unity.o
	$(LINK) -o $@ $^

# Compile les fichier .c du repertoire de test en .o
# :: permet de créer une régle indépendante, sans ça seule la dernière règle serait exécutée.
$(PATH_OBJ)%.o:: $(PATH_TEST)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATH_OBJ)%.o:: $(SERVER) 
	$(COMPILE) $(CFLAGS) $< -o $@

# dernière règle pour $(PATH_OBJ)%.o
$(PATH_OBJ)%.o:: $(PATH_UNITY)%.c $(PATH_UNITY)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATH_DEP)%.d:: $(PATH_TEST)%.c
	$(DEPEND) $@ $<

$(PATH_BUILD):
	$(MKDIR) $(PATH_BUILD)

$(PATH_DEP):
	$(MKDIR) $(PATH_DEP)

$(PATH_OBJ):
	$(MKDIR) $(PATH_OBJ)

$(PATH_RES):
	$(MKDIR) $(PATH_RES)

clean:
	$(CLEANUP) $(PATH_OBJ)*.o
	$(CLEANUP) $(PATH_BUILD)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATH_RES)*.txt

.PRECIOUS: $(PATH_BUILD)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_DEP)%.d
.PRECIOUS: $(PATH_OBJ)%.o
.PRECIOUS: $(PATH_RES)%.txt
