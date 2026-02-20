CFLAG_VERSION = -std=c2x
EXT_DEP= -lpcap -lsqlite3
TEST_PREFIX = test_

PATH_UNITY = unity/src/
PATH_LIB = lib/
PATH_TEST = test/
PATH_BUILD = build/
PATH_BUILD_TEST = build/test/
PATH_BUILD_TEST_OBJ = build/test/objs/
PATH_BUILD_TEST_RES = build/test/results/
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
	clang $(CFLAG_VERSION) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
	
client:$(CLIENT_WITH_MAIN)
	clang $(CFLAG_VERSION) $(CLIENT_WITH_MAIN) -o client

###########  SANITIZERS/DEBUGGING  ###########


# Generates a list of object file paths (.o) corresponding to server files (.c)
# by changing the path to point to 'build/sanitizer/objs/' while preserving the original directory structure.
# (e.g., 'lib/command/cmd_builder.c' becomes 'build/sanitizer/objs/command/cmd_builder.o')
OBJLIB_SERVER := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ_SANITIZER)%.o,$(SERVER_LIB)) $(PATH_BUILD_SANITIZER)main.o
OBJLIB_CLIENT := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ_SANITIZER)%.o,$(CLIENT_LIB)) $(PATH_BUILD_SANITIZER)client.o

# $(info OBJLIB_SERVER = '$(OBJLIB_SERVER)')

debug_server_sanitizer:$(PATH_BUILD_SANITIZER)main
	ASAN_OPTIONS=detect_leaks=1 $(PATH_BUILD_SANITIZER)main
	
debug_client_sanitizer:$(PATH_BUILD_SANITIZER)client
	ASAN_OPTIONS=detect_leaks=1 $(PATH_BUILD_SANITIZER)client

$(PATH_BUILD_SANITIZER)main:$(OBJLIB_SERVER)
	@echo "\n[Linking for ]"
	clang -g -O0 -fsanitize=address,leak $(CFLAG_VERSION) -o $@ $^ $(EXT_DEP) 
	
$(PATH_BUILD_SANITIZER)client:$(OBJLIB_CLIENT)
	@echo "\n[Linking]"
	clang -g -O0 -fsanitize=address,leak $(CFLAG_VERSION) -o $@ $^ $(EXT_DEP) 

$(PATH_BUILD_OBJ_SANITIZER)%.o: $(PATH_LIB)%.c
	@echo "\n[Compiling source files for debugging]"
	@mkdir -p $(dir $@)
	clang -g -O0 -fsanitize=address,leak -c $(CFLAG_VERSION)  $< -o $@ 
	
$(PATH_BUILD_SANITIZER)main.o: ./main.c
	@echo "\n[Compiling main file for debugging]"
	@mkdir -p $(dir $@)
	clang -g -O0 -fsanitize=address,leak -c $(CFLAG_VERSION)  $< -o $@ 	

$(PATH_BUILD_SANITIZER)client.o: ./client.c
	@echo "\n[Compiling client file for debugging]"
	@mkdir -p $(dir $@)
	clang -g -O0 -fsanitize=address,leak -c $(CFLAG_VERSION)  $< -o $@ 


# run_client_with_san:client_with_san
# 	./client

# main_with_san:$(OBJLIB_SERVER)
# 	clang -g -O0 -fsanitize=address -c $(CFLAG_VERSION) $(SERVER_WITH_MAIN) $(EXT_DEP) -o main
	
# client_with_san:$(OBJLIB_CLIENT)
# 	clang -g -O0 -fsanitize=address -c $(CFLAG_VERSION) $(CLIENT_WITH_MAIN) -o client


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

PASSED = `grep -s PASS $(PATH_BUILD_TEST_RES)*.txt`
FAIL = `grep -s FAIL $(PATH_BUILD_TEST_RES)*.txt`
IGNORE = `grep -s IGNORE $(PATH_BUILD_TEST_RES)*.txt`


CFLAGS_TEST=$(CFLAG_VERSION) -I$(PATH_UNITY)
COMPILE=clang -g -O0 -c $(CFLAGS_TEST)
LINK=clang

# COMPILE=clang -g -O0 -c $(CFLAGS_TEST)
# LINK=clang

SRC_TEST_PATHS := $(shell find $(PATH_LIB) -type f -name "$(TEST_PREFIX)*.c")
RUNNER_NAME = test_runner

SRCLIB_WITH_TESTS := $(SERVER_LIB) $(CLIENT_LIB) $(SRC_TEST_PATHS) $(PATH_LIB)$(RUNNER_NAME).c
OBJLIB_WITH_TESTS := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_TEST_OBJ)%.o,$(SRCLIB_WITH_TESTS))

BUILD_PATHS = $(PATH_BUILD) $(PATH_BUILD_TEST) $(PATH_BUILD_TEST_DEP) $(PATH_BUILD_TEST_OBJ) $(PATH_BUILD_TEST_RES)

RUNNER_RESULT = $(PATH_BUILD_TEST_RES)$(RUNNER_NAME).txt

clean_test: clean test 

test: $(BUILD_PATHS) $(RUNNER_RESULT)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# 'build/results/test/test_runner.txt' : build/test/test_runner.out
$(RUNNER_RESULT): $(PATH_BUILD_TEST)$(RUNNER_NAME).$(TARGET_EXTENSION)
	@echo "\n[Execute $*.$(TARGET_EXTENSION)]"
	./$< > $@ 2>&1
# 	-./$< > $@ 2>&1
# 	./$< 2>&1 | tee $@


# build/test_runner.out
$(PATH_BUILD_TEST)$(RUNNER_NAME).$(TARGET_EXTENSION): \
	$(OBJLIB_WITH_TESTS) \
	$(PATH_BUILD_TEST_OBJ)$(RUNNER_NAME).o \
	$(PATH_BUILD_TEST_OBJ)unity.o
	@echo "\n[Linking for $*]"
	$(LINK) -o $@ $^ $(EXT_DEP)

# Compile the source files (.c) to OBJLIB_WITH_TESTS files (.o)
$(PATH_BUILD_TEST_OBJ)%.o: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILE) $< -o $@

# Compile test/test_runner.c to build/test/objs/test_runner.o
$(PATH_BUILD_TEST_OBJ)$(RUNNER_NAME).o: $(PATH_TEST)$(RUNNER_NAME).c
	@echo "\n[Compiling test runner file]"
	$(COMPILE) $< -o $@

# Compile the Unity framework files
$(PATH_BUILD_TEST_OBJ)unity.o: $(PATH_UNITY)unity.c $(PATH_UNITY)unity.h
	@echo "\n[Compiling Unity files] $< -> $@"
	$(COMPILE) $< -o $@

$(PATH_BUILD):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD)
	
$(PATH_BUILD_TEST):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD_TEST)

$(PATH_BUILD_TEST_OBJ):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD_TEST_OBJ)
	
$(PATH_BUILD_TEST_RES):
	@echo "Creating directory: $@"
	$(MKDIR) $(PATH_BUILD_TEST_RES)

clean:
	$(CLEANUP) $(PATH_BUILD)*
	$(CLEANUP) $(PATH_BUILD_TEST)*
	$(CLEANUP) $(PATH_BUILD_TEST_OBJ)*
	$(CLEANUP) $(PATH_BUILD_TEST_RES)*.txt	
	$(CLEANUP) $(PATH_BUILD_SANITIZER)*
	$(CLEANUP) $(PATH_BUILD_OBJ_SANITIZER)*


.PRECIOUS: $(PATH_BUILD_TEST_RES)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATH_BUILD_TEST_OBJ)%.o
.PRECIOUS: $(PATH_BUILD_TEST_RES)