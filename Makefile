COMPILER=clang
CFLAG_VERSION = -std=c2x
CFLAG_DEBUG = -g -O0
CFLAG_SAN = -fsanitize=address,leak
EXT_DEP= -lpcap -lsqlite3

DEBUG_SAN_FLAGS= $(CFLAG_DEBUG) $(CFLAG_SAN) $(CFLAG_VERSION)

TEST_PREFIX = test_
LEAK_OPTION = ASAN_OPTIONS=detect_leaks=1

PATH_UNITY = unity/src/
PATH_LIB = lib/
PATH_RUNNER = test_runner/
PATH_BUILD_TEST = build/test/
PATH_BUILD_TEST_OBJ = build/test/objs/
PATH_BUILD_SAN = build/sanitizer/
PATH_BUILD_SAN_OBJ = build/sanitizer/objs/

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

OBJLIB_SERVER = $(addprefix $(PATH_BUILD_SAN_OBJ),$(SERVER_WITH_MAIN:.c=.o))
OBJLIB_CLIENT = $(addprefix $(PATH_BUILD_SAN_OBJ),$(CLIENT_WITH_MAIN:.c=.o))

# $(info OBJLIB_SERVER = '$(OBJLIB_SERVER)')

debug_server_sanitizer:$(PATH_BUILD_SAN)main
	$(LEAK_OPTION) $(PATH_BUILD_SAN)main
	
debug_client_sanitizer:$(PATH_BUILD_SAN)client
	$(LEAK_OPTION) $(PATH_BUILD_SAN)client

$(PATH_BUILD_SAN)main:$(OBJLIB_SERVER)
	@echo "\n[SANITIZERS/DEBUGGING: Linking for server]"
	$(COMPILER) $(DEBUG_SAN_FLAGS) $(EXT_DEP) -o $@ $^  
	
$(PATH_BUILD_SAN)client:$(OBJLIB_CLIENT)
	@echo "\n[SANITIZERS/DEBUGGING: Linking for client]"
	$(COMPILER) $(DEBUG_SAN_FLAGS) $(EXT_DEP) -o $@ $^

$(PATH_BUILD_SAN_OBJ)%.o: %.c
	@echo "\n[SANITIZERS/DEBUGGING: Compiling source files] $< -> $@"
	@mkdir -p $(dir $@)
	$(COMPILER) $(DEBUG_SAN_FLAGS) -c $< -o $@
	
###########  TESTING  ###########

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

COMPILE_FOR_TESTS=$(COMPILER) $(CFLAG_DEBUG) -c $(CFLAG_VERSION) -I$(PATH_UNITY)
LINK=$(COMPILER)

RUNNER_NAME = test_runner

TEST_FILES := $(shell find $(PATH_LIB) -type f -name "$(TEST_PREFIX)*.c") $(PATH_RUNNER)$(RUNNER_NAME).c
SRCLIB_WITH_TESTS := $(SERVER_LIB) $(CLIENT_LIB) $(TEST_FILES) 
OBJLIB_WITH_TESTS = $(addprefix $(PATH_BUILD_TEST_OBJ),$(SRCLIB_WITH_TESTS:.c=.o))

RESULT_TXT = $(PATH_BUILD_TEST)$(RUNNER_NAME).txt
RESULT_EXE = $(PATH_BUILD_TEST)$(RUNNER_NAME).$(TARGET_EXTENSION)

PASSED = `grep -s PASS $(RESULT_TXT)`
FAIL = `grep -s FAIL $(RESULT_TXT)`
IGNORE = `grep -s IGNORE $(RESULT_TXT)`

clean_test: clean test 

test: $(RESULT_TXT)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

$(RESULT_TXT): $(RESULT_EXE)
	@echo "\n[TESTING: Execute $*.$(TARGET_EXTENSION)]"
	./$< > $@ 2>&1
# 	-./$< > $@ 2>&1
# 	./$< 2>&1 | tee $@

$(RESULT_EXE): \
	$(OBJLIB_WITH_TESTS) \
	$(PATH_BUILD_TEST_OBJ)unity.o
	@echo "\n[TESTING: Linking for $*]"
	$(LINK) -o $@ $^ $(EXT_DEP)

$(PATH_BUILD_TEST_OBJ)%.o: %.c
	@echo "\n[TESTING: Compiling source files] $< -> $@"
	@mkdir -p $(dir $@)
	$(COMPILE_FOR_TESTS) $< -o $@

$(PATH_BUILD_TEST_OBJ)unity.o: $(PATH_UNITY)unity.c $(PATH_UNITY)unity.h
	@echo "\n[TESTING: Compiling Unity files] $< -> $@"
	$(COMPILE_FOR_TESTS) $< -o $@

clean:
	$(CLEANUP) $(PATH_BUILD_TEST)*
	$(CLEANUP) $(PATH_BUILD_TEST_OBJ)*
	$(CLEANUP) $(PATH_BUILD_SAN)*
	$(CLEANUP) $(PATH_BUILD_SAN_OBJ)*
