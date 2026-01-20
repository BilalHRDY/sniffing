CFLAG_VERSION = -std=c2x
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

###########  CREATE EXECUTABLES AND LAUNCH ###########

run:main
	./main
	
run_client:client
	./client

main:$(SERVER_WITH_MAIN)
	clang $(CFLAG_VERSION) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
	
client:$(CLIENT_WITH_MAIN)
	clang $(CFLAG_VERSION) $(CLIENT_WITH_MAIN) -o client


###########  DEBUGGING ###########

debug_main:main_d
	lldb ./main

main_d:$(SERVER_WITH_MAIN)
	clang -g -O0 $(CFLAG_VERSION) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
    
# client debug
debug_client:client_d
	lldb ./client

client_d:$(CLIENT_WITH_MAIN)
	clang -g -O0 $(CFLAG_VERSION) $(CLIENT_WITH_MAIN) -o client


###########  TESTING ###########

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

# Generates a list of object file paths (.o) corresponding to all source files (.c)
# by changing the path to point to 'build/objs/' while preserving the original directory structure.
# (e.g., 'lib/command/cmd_builder.c' becomes 'build/objs/command/cmd_builder.o')
OBJLIB := $(patsubst $(PATH_LIB)%.c,$(PATH_OBJ)%.o,$(SRCLIB))
# $(info OBJLIB = '$(OBJLIB)')

BUILD_PATHS = $(PATH_BUILD) $(PATH_DEP) $(PATH_OBJ) $(PATH_RES)

# Generates a list of all test file paths currently present in /test.
SRCT = $(wildcard $(PATH_TEST)*.c)

CFLAGS_TEST=$(CFLAG_VERSION) -I$(PATH_UNITY)
COMPILE=clang -c $(CFLAGS_TEST)
LINK=clang
# DEPEND=gcc -MM -MG -MF
# CFLAGS=-I$(PATH_UNITY)

# Generates a list of file paths (.txt) corresponding to all test files
# by changing the path to point to 'build/results/'
# (e.g., 'test/Teststring_helpers.c' becomes 'build/results/Teststring_helpers.txt')
TEST_RESULTS_FILES = $(patsubst $(PATH_TEST)Test_%.c,$(PATH_RES)Test_%.txt,$(SRCT) )

test: $(BUILD_PATHS) $(TEST_RESULTS_FILES)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

$(PATH_RES)%.txt: $(PATH_BUILD)%.$(TARGET_EXTENSION)
# runs 'Testcalc.out' and redirects output to 'Testcalc.txt'
	-./$< > $@ 2>&1

$(PATH_BUILD)Test_%.$(TARGET_EXTENSION): \
	$(PATH_OBJ)Test_%.o \
	$(OBJLIB) \
	$(PATH_OBJ)unity.o
	@echo "\n[Linking for $@] "
	$(LINK) -o $@ $^ $(EXT_DEP)

# Compile the test files (.c) to object files (.o)
# "::" allows an independent rule; otherwise only the last $(PATH_OBJ)%.o rule would run.
$(PATH_OBJ)%.o:: $(PATH_TEST)%.c
	@echo "\n[Compiling tests] $@"
	$(COMPILE) $< -o $@

# Compile the source files (.c) to object files (.o)
$(PATH_OBJ)%.o:: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILE) $< -o $@


# Compile the Unity framework files
$(PATH_OBJ)%.o:: $(PATH_UNITY)%.c $(PATH_UNITY)%.h
	@echo "\n[Compiling Unity files] $< -> $@"
	$(COMPILE) $< -o $@


# $(PATH_DEP)%.d:: $(PATH_TEST)%.c
# 	@echo "\n[CC] $< -> $@"
# 	$(DEPEND) $@ $<

$(PATH_BUILD):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD)

$(PATH_DEP):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_DEP)

$(PATH_OBJ):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_OBJ)
	
$(PATH_RES):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_RES)

clean:
	$(CLEANUP) $(PATH_OBJ)*
	$(CLEANUP) $(PATH_BUILD)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATH_RES)*.txt

.PRECIOUS: $(PATH_BUILD)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_DEP)%.d
.PRECIOUS: $(PATH_OBJ)%.o
.PRECIOUS: $(PATH_RES)%.txt