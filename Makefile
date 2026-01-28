CFLAG_VERSION = -std=c2x
EXT_DEP= -lpcap -lsqlite3
TEST_PREFIX = test_

# Be careful, '**/*' is not recursive
SERVER_LIB = \
    $(wildcard lib/*.c) \
    $(wildcard lib/command/*.c) \
    $(wildcard lib/utils/*.c) \
    $(wildcard lib/utils/calc/*.c) \
    $(wildcard lib/utils/string/*.c) \
    $(wildcard lib/utils/data_structures/*.c) \
    $(wildcard lib/ipc/socket/server/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \

# filter out tests
SERVER_LIB := $(shell \
	printf "%s\n" $(SERVER_LIB) | grep -v '/$(TEST_PREFIX)[^/]*\.c$$' \
)


$(info SERVER_LIB = '$(SERVER_LIB)')
SERVER_WITH_MAIN = main.c $(SERVER_LIB)


CLIENT_LIB = \
    lib/command/cmd_serializer.c \
    lib/command/cmd_builder.c \
    $(wildcard lib/utils/string/*.c) \
    $(wildcard lib/ipc/socket/client/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \
    $(wildcard lib/client/*.c) \

# filter out tests
CLIENT_LIB := $(shell \
	printf "%s\n" $(CLIENT_LIB) | grep -v '/$(TEST_PREFIX)[^/]*\.c$$' \
)
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

PASSED = `grep -s PASS $(PATH_BUILD_RES)*.txt`
FAIL = `grep -s FAIL $(PATH_BUILD_RES)*.txt`
IGNORE = `grep -s IGNORE $(PATH_BUILD_RES)*.txt`

PATH_UNITY = unity/src/
PATH_LIB = lib/
PATH_TEST = test/
PATH_BUILD = build/
PATH_BUILD_OBJ = build/objs/
PATH_BUILD_RES = build/results/
PATH_DEP = build/depends/

CFLAGS_TEST=$(CFLAG_VERSION) -I$(PATH_UNITY)
COMPILE=clang -g -O0 -c $(CFLAGS_TEST)
LINK=clang

SRC_TEST_PATHS := $(shell find $(PATH_LIB) -type f -name "$(TEST_PREFIX)*.c")
RUNNER_NAME = test_runner

SRCLIB := $(SERVER_LIB) $(CLIENT_LIB) $(SRC_TEST_PATHS) $(PATH_LIB)$(RUNNER_NAME).c

# Generates a list of object file paths (.o) corresponding to all source files (.c)
# by changing the path to point to 'build/objs/' while preserving the original directory structure.
# (e.g., 'lib/command/cmd_builder.c' becomes 'build/objs/command/cmd_builder.o')
OBJLIB := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ)%.o,$(SRCLIB))

BUILD_PATHS = $(PATH_BUILD) $(PATH_DEP) $(PATH_BUILD_OBJ) $(PATH_BUILD_RES)

RUNNER_RESULT = $(PATH_BUILD_RES)$(RUNNER_NAME).txt

clean_test: clean test 

test: $(BUILD_PATHS) $(RUNNER_RESULT)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# 'build/results/test_runner.txt' : build/test_runner.out
$(RUNNER_RESULT): $(PATH_BUILD)$(RUNNER_NAME).$(TARGET_EXTENSION)
	@echo "\n[Execute $*.$(TARGET_EXTENSION)]"
	-./$< > $@ 2>&1

# build/test_runner.out
$(PATH_BUILD)$(RUNNER_NAME).$(TARGET_EXTENSION): \
	$(OBJLIB) \
	$(PATH_BUILD_OBJ)$(RUNNER_NAME).o \
	$(PATH_BUILD_OBJ)$(RUNNER_NAME).o \
	$(PATH_BUILD_OBJ)unity.o
	@echo "\n[Linking for $*]"
	$(LINK) -o $@ $^ $(EXT_DEP)

# Compile the source files (.c) to object files (.o)
$(PATH_BUILD_OBJ)%.o: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILE) $< -o $@

# Compile test/test_runner.c to build/objs/test_runner.o
$(PATH_BUILD_OBJ)$(RUNNER_NAME).o: $(PATH_TEST)$(RUNNER_NAME).c
	@echo "\n[Compiling test runner file]"
	$(COMPILE) $< -o $@

# Compile the Unity framework files
$(PATH_BUILD_OBJ)unity.o: $(PATH_UNITY)unity.c $(PATH_UNITY)unity.h
	@echo "\n[Compiling Unity files] $< -> $@"
	$(COMPILE) $< -o $@

$(PATH_BUILD):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD)

$(PATH_DEP):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_DEP)

$(PATH_BUILD_OBJ):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD_OBJ)
	
$(PATH_BUILD_RES):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD_RES)

clean:
	$(CLEANUP) $(PATH_BUILD_OBJ)*
	$(CLEANUP) $(PATH_BUILD)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATH_BUILD_RES)*.txt

.PRECIOUS: $(PATH_BUILD_RES)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_DEP)%.d
.PRECIOUS: $(PATH_BUILD_OBJ)%.o
.PRECIOUS: $(PATH_BUILD_RES)