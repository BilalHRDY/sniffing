COMPILER=clang
CFLAG_VERSION = -std=c2x
CFLAG_DEBUG = -g -O0
CFLAG_SAN = -fsanitize=address,leak
EXT_DEP= -lpcap -lsqlite3

TEST_PREFIX = test_
LEAK_OPTION = ASAN_OPTIONS=detect_leaks=1

PATH_UNITY = unity/src/
PATH_LIB = lib/
PATH_TEST = test/
PATH_BUILD_TEST = build/test/
PATH_BUILD_TEST_OBJ = build/test/objs/
PATH_BUILD_SANITIZER = build/sanitizer/
PATH_BUILD_OBJ_SANITIZER = build/sanitizer/objs/

# Be careful, '**/*' pattern is not recursive
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

# $(info SERVER_LIB = '$(SERVER_LIB)')
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

run_server:main
	./main
	
run_client:client
	./client

main:$(SERVER_WITH_MAIN)
	$(COMPILER) $(CFLAG_VERSION) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
	
client:$(CLIENT_WITH_MAIN)
	$(COMPILER) $(CFLAG_VERSION) $(CLIENT_WITH_MAIN) -o client

###########  SANITIZERS/DEBUGGING  ###########


# Generates a list of object file paths (.o) corresponding to server files (.c)
# by changing the path to point to 'build/sanitizer/objs/' while preserving the original directory structure.
# (e.g., 'lib/command/cmd_builder.c' becomes 'build/sanitizer/objs/command/cmd_builder.o')
OBJLIB_SERVER := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ_SANITIZER)%.o,$(SERVER_LIB)) $(PATH_BUILD_SANITIZER)main.o
OBJLIB_CLIENT := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ_SANITIZER)%.o,$(CLIENT_LIB)) $(PATH_BUILD_SANITIZER)client.o

# $(info OBJLIB_SERVER = '$(OBJLIB_SERVER)')

debug_server_sanitizer:$(PATH_BUILD_SANITIZER)main
	$(LEAK_OPTION) $(PATH_BUILD_SANITIZER)main
	
debug_client_sanitizer:$(PATH_BUILD_SANITIZER)client
	$(LEAK_OPTION) $(PATH_BUILD_SANITIZER)client

$(PATH_BUILD_SANITIZER)main:$(OBJLIB_SERVER)
	@echo "\n[Linking for server]"
	$(COMPILER) $(CFLAG_DEBUG) $(CFLAG_SAN) $(CFLAG_VERSION) -o $@ $^ $(EXT_DEP) 
	
$(PATH_BUILD_SANITIZER)client:$(OBJLIB_CLIENT)
	@echo "\n[Linking for client]"
	$(COMPILER) $(CFLAG_DEBUG) $(CFLAG_SAN) $(CFLAG_VERSION) -o $@ $^ $(EXT_DEP) 

$(PATH_BUILD_OBJ_SANITIZER)%.o: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAG_DEBUG) $(CFLAG_SAN) -c $(CFLAG_VERSION)  $< -o $@ 
	
$(PATH_BUILD_SANITIZER)main.o: ./main.c
	@echo "\n[Compiling main file]"
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAG_DEBUG) $(CFLAG_SAN) -c $(CFLAG_VERSION)  $< -o $@ 	

$(PATH_BUILD_SANITIZER)client.o: ./client.c
	@echo "\n[Compiling client file]"
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAG_DEBUG) $(CFLAG_SAN) -c $(CFLAG_VERSION)  $< -o $@ 


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


COMPILE_FOR_TESTS=$(COMPILER) $(CFLAG_DEBUG) -c $(CFLAG_VERSION) -I$(PATH_UNITY)
LINK=$(COMPILER)

SRC_TEST_PATHS := $(shell find $(PATH_LIB) -type f -name "$(TEST_PREFIX)*.c")
RUNNER_NAME = test_runner

SRCLIB_WITH_TESTS := $(SERVER_LIB) $(CLIENT_LIB) $(SRC_TEST_PATHS) $(PATH_LIB)$(RUNNER_NAME).c
OBJLIB_WITH_TESTS := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_TEST_OBJ)%.o,$(SRCLIB_WITH_TESTS))

RUNNER_TXT = $(PATH_BUILD_TEST)$(RUNNER_NAME).txt
RUNNER_EXE = $(PATH_BUILD_TEST)$(RUNNER_NAME).$(TARGET_EXTENSION)

PASSED = `grep -s PASS $(RUNNER_TXT)`
FAIL = `grep -s FAIL $(RUNNER_TXT)`
IGNORE = `grep -s IGNORE $(RUNNER_TXT)`

clean_test: clean test 

test: $(RUNNER_TXT)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# 'build/results/test/test_runner.txt' : build/test/test_runner.out
$(RUNNER_TXT): $(RUNNER_EXE)
	@echo "\n[Execute $*.$(TARGET_EXTENSION)]"
	./$< > $@ 2>&1
# 	-./$< > $@ 2>&1
# 	./$< 2>&1 | tee $@

$(RUNNER_EXE): \
	$(OBJLIB_WITH_TESTS) \
	$(PATH_BUILD_TEST_OBJ)$(RUNNER_NAME).o \
	$(PATH_BUILD_TEST_OBJ)unity.o
	@echo "\n[Linking for $*]"
	$(LINK) -o $@ $^ $(EXT_DEP)

$(PATH_BUILD_TEST_OBJ)%.o: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILE_FOR_TESTS) $< -o $@

$(PATH_BUILD_TEST_OBJ)$(RUNNER_NAME).o: $(PATH_TEST)$(RUNNER_NAME).c
	@echo "\n[Compiling test runner file]"
	$(COMPILE_FOR_TESTS) $< -o $@

$(PATH_BUILD_TEST_OBJ)unity.o: $(PATH_UNITY)unity.c $(PATH_UNITY)unity.h
	@echo "\n[Compiling Unity files] $< -> $@"
	$(COMPILE_FOR_TESTS) $< -o $@

clean:
	$(CLEANUP) $(PATH_BUILD_TEST)*
	$(CLEANUP) $(PATH_BUILD_TEST_OBJ)*
	$(CLEANUP) $(PATH_BUILD_SANITIZER)*
	$(CLEANUP) $(PATH_BUILD_OBJ_SANITIZER)*


.PRECIOUS: $(PATH_BUILD_TEST_RES)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_BUILD_TEST_OBJ)%.o
.PRECIOUS: $(PATH_BUILD_TEST_RES)