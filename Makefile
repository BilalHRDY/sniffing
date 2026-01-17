CFLAGS = -std=c2x
EXT_DEP= -lpcap -lsqlite3
SERVER_LIB = \
    $(wildcard lib/*.c) \
    $(wildcard lib/command/*.c) \
    $(wildcard lib/utils/*.c) \
    $(wildcard lib/utils/**/*.c) \
    $(wildcard lib/ipc/socket/server/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \

SERVER_WITH_MAIN = main.c $(SERVER_LIB)

CLIENT_LIB = \
    lib/command/cmd_serializer.c \
    lib/command/cmd_builder.c \
    $(wildcard lib/utils/string/*.c) \
    $(wildcard lib/ipc/socket/client/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \
    $(wildcard lib/client/*.c) \
    $(wildcard lib/client/**/*.c) \


CLIENT_WITH_MAIN = client.c $(CLIENT_LIB)


run:main
	./main
	
run_client:client
	./client

main:$(SERVER_WITH_MAIN)
	clang $(CFLAGS) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
	
client:$(CLIENT_WITH_MAIN)
	clang $(CFLAGS) $(CLIENT_WITH_MAIN) -o client

# server debug
debug_main:main_d
	lldb ./main

main_d:$(SERVER_WITH_MAIN)
	clang -g -O0 $(CFLAGS) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
    
# client debug
debug_client:client_d
	lldb ./client

client_d:$(CLIENT_WITH_MAIN)
	clang -g -O0 $(CFLAGS) $(CLIENT_WITH_MAIN) -o client


###########  TEST ###########

ifeq ($(OS),Windows_NT)
# Create the equivalent of rm -rf for Windows.
  ifeq ($(shell uname -s),) # not in a bash-like shell
    CLEANUP = del /F /Q
    MKDIR = mkdir
  else # in a bash-like shell, like msys
    CLEANUP = rm -rf
    MKDIR = mkdir -p
  endif
    TARGET_EXTENSION:=exe
else
  CLEANUP = rm -rf
  MKDIR = mkdir -p
  TARGET_EXTENSION=out
endif


.PHONY: clean
.PHONY: test

PASSED = `grep -s PASS $(PATH_RES)*.txt`
FAIL = `grep -s FAIL $(PATH_RES)*.txt`
IGNORE = `grep -s IGNORE $(PATH_RES)*.txt`

PATH_UNITY = unity/src/
PATH_LIB = lib/
PATH_TEST = test/
PATH_BUILD = build/
PATH_DEP = build/depends/
PATH_OBJ = build/objs/
PATH_RES = build/results/

SRCLIB := $(SERVER_LIB) $(CLIENT_LIB)

# Génère la liste des fichiers objets (.o) correspondant à tous les fichiers sources (.c) 
# en modifiant le chemin pour pointer vers build/objs/ en respectant l'arborescence initiale.
# (ex: lib/command/cmd_builder.c) est transformé en son équivalent build/objs/command/cmd_builder.o
OBJLIB := $(patsubst $(PATH_LIB)%.c,$(PATH_OBJ)%.o,$(SRCLIB))
$(info OBJLIB = '$(OBJLIB)')

BUILD_PATHS = $(PATH_BUILD) $(PATH_DEP) $(PATH_OBJ) $(PATH_RES)

# test/Testcalc.c
SRCT = $(wildcard $(PATH_TEST)*.c)

CFLAGS_DEBUG=-std=c2x -I$(PATH_UNITY)
COMPILE=gcc -c $(CFLAGS_DEBUG)
LINK=gcc
DEPEND=gcc -MM -MG -MF
# CFLAGS=-I$(PATH_UNITY)

# Pour test/Testcalc.c on a build/results/Testcalc.txt
RESULTS = $(patsubst $(PATH_TEST)Test%.c,$(PATH_RES)Test%.txt,$(SRCT) )

# test: build/ build/depends/ build/objs/ build/results/ build/results/Test_string_helpers.txt
test: $(BUILD_PATHS)  $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# compilation: $(OBJLIB)

# build/results/Test_string_helpers.txt: build/Test_string_helpers.out
$(PATH_RES)%.txt: $(PATH_BUILD)%.$(TARGET_EXTENSION)
# exécute Testcalc.out - redirige la sortie vers Testcalc.txt
	-./$< > $@ 2>&1


$(PATH_BUILD)Test%.$(TARGET_EXTENSION): \
	$(PATH_OBJ)Test%.o \
	$(OBJLIB) \
	$(PATH_OBJ)unity.o
	@echo "\n[Création de liens pour $@] "
# 	$(info $ $(PATH_BUILD)Test%.$(TARGET_EXTENSION): $(LINK) -o $@ $^)
	$(LINK) -o $@ $^ $(EXT_DEP)


# Compile les fichiers .c du repertoire de test en .o
# "::" permet de créer une régle indépendante, sans ça seule la dernière règle $(PATH_OBJ)%.o serait exécutée.
$(PATH_OBJ)%.o:: $(PATH_TEST)%.c
	@echo "\n[compilation des tests:] $@"
# 	@echo "[CC] $< -> $@"
# 	$(info $ $(PATH_OBJ)%.o:: $(PATH_TEST)%.c: $(COMPILE) $< -o $@)
	$(COMPILE) $< -o $@

# Compile les fichiers .c des fichiers sources en .o
$(PATH_OBJ)%.o:: $(PATH_LIB)%.c
# 	@echo "[compilation des fichiers sources] $< -> $@"
	@echo "\n[compilation des fichiers sources:]"
	@mkdir -p $(dir $@)
# 	$(info $ $(PATH_OBJ)%.o:: $(PATH_LIB)%.c: $(COMPILE) $< -o $@)
	$(COMPILE) $< -o $@


# dernière règle pour $(PATH_OBJ)%.o
$(PATH_OBJ)%.o:: $(PATH_UNITY)%.c $(PATH_UNITY)%.h
	@echo "\n[compilation des fichiers unity:] $< -> $@"
	$(COMPILE) $< -o $@


# $(PATH_DEP)%.d:: $(PATH_TEST)%.c
# 	@echo "\n[CC] $< -> $@"
# 	$(DEPEND) $@ $<

$(PATH_BUILD):
	@echo "Creation du repertoire: $@"
	$(MKDIR) $(PATH_BUILD)

$(PATH_DEP):
	@echo "Creation du repertoire: $@"
	$(MKDIR) $(PATH_DEP)

$(PATH_OBJ):
	@echo "Creation du repertoire: $@"
	$(MKDIR) $(PATH_OBJ)
	
$(PATH_RES):
	@echo "Creation du repertoire: $@"
	$(MKDIR) $(PATH_RES)

clean:
	$(CLEANUP) $(PATH_OBJ)*
	$(CLEANUP) $(PATH_BUILD)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATH_RES)*.txt

.PRECIOUS: $(PATH_BUILD)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_DEP)%.d
.PRECIOUS: $(PATH_OBJ)%.o
.PRECIOUS: $(PATH_RES)%.txt
