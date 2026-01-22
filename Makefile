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
    $(wildcard lib/ipc/socket/server/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \


# SERVER_LIB := $(filter-out %/$(TEST_PREFIX)%.c,$(SERVER_LIB))
$(info SERVER_LIB = '$(SERVER_LIB)')
SERVER_WITH_MAIN = main.c $(SERVER_LIB)


CLIENT_LIB = \
    lib/command/cmd_serializer.c \
    lib/command/cmd_builder.c \
    $(wildcard lib/utils/string/*.c) \
    $(wildcard lib/ipc/socket/client/*.c) \
    $(wildcard lib/ipc/protocol/*.c) \
    $(wildcard lib/client/*.c) \

# CLIENT_LIB := $(filter-out %/$(TEST_PREFIX)%.c,$(CLIENT_LIB))
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
# PATH_TEST = test/
PATH_BUILD = build/
PATH_BUILD_OBJ = build/objs/
PATH_BUILD_RES = build/results/
PATH_DEP = build/depends/

CFLAGS_TEST=$(CFLAG_VERSION) -I$(PATH_UNITY)
COMPILE=clang -c $(CFLAGS_TEST)
LINK=clang

SRC_TEST_PATHS := $(shell find $(PATH_LIB) -type f -name "$(TEST_PREFIX)*.c")
SRC_TEST_NAMES := $(notdir $(SRC_TEST_PATHS))

SRCLIB := $(SERVER_LIB) $(CLIENT_LIB) $(SRC_TEST_PATHS)

# Generates a list of object file paths (.o) corresponding to all source files (.c)
# by changing the path to point to 'build/objs/' while preserving the original directory structure.
# (e.g., 'lib/command/cmd_builder.c' becomes 'build/objs/command/cmd_builder.o')
OBJLIB := $(patsubst $(PATH_LIB)%.c,$(PATH_BUILD_OBJ)%.o,$(SRCLIB))

BUILD_PATHS = $(PATH_BUILD) $(PATH_DEP) $(PATH_BUILD_OBJ) $(PATH_BUILD_RES)

# Generates a list of file paths (.txt) corresponding to all test files
# by changing the path to point to 'build/results/'
# (e.g., 'utils/string/tests/test_string_helpers.txt.c' becomes 'build/results/test_string_helpers.txt.txt')
TEST_RESULTS_TXT := $(patsubst %.c,$(PATH_BUILD_RES)%.txt,$(SRC_TEST_NAMES))

test: $(BUILD_PATHS) $(TEST_RESULTS_TXT)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# 'build/results/test_string_helpers.txt' : build/results/test_string_helpers.out
%.txt: %.$(TARGET_EXTENSION)
# runs 'Testcalc.out' and redirects output to 'Testcalc.txt'
	@echo "\n[execute .out] "
	mkdir -p $(dir $@)
	-./$< > $@ 2>&1

# build/results/test_string_helpers.out
%.$(TARGET_EXTENSION): \
	$(OBJLIB) \
	$(PATH_BUILD_OBJ)unity.o
	@echo "\n[Linking for $@] "
	mkdir -p $(dir $@)
	$(LINK) -o $@ $^ $(EXT_DEP)

# Compile the source files (.c) to object files (.o)
$(PATH_BUILD_OBJ)%.o:: $(PATH_LIB)%.c
	@echo "\n[Compiling source files]"
	@mkdir -p $(dir $@)
	$(COMPILE) $< -o $@

# Compile the Unity framework files
$(PATH_BUILD_OBJ)%.o:: $(PATH_UNITY)%.c $(PATH_UNITY)%.h
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
.PRECIOUS: $(PATH_BUILD_RES)%.txt