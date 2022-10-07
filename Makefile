EXECUTABLE_NAME = sandbox

BUILD_DIR = build
SRC_DIR = src
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
PROGRAM = $(BIN_DIR)/$(EXECUTABLE_NAME)

CC = gcc
LD = gcc

CFLAGS = -g -Wall -std=c99 -pie -fPIC
LDFLAGS = -L/usr/local/lib/ -Wl,-R/usr/local/lib/
LIBS = -lseccomp 

# include src and all subdirectories using vpath
vpath %.c  $(shell find $(SRC_DIR) -type d)
CFILES = $(shell find $(SRC_DIR) -name "*.c" | xargs -n 1 basename)
OFILES = $(CFILES:%.c=$(OBJ_DIR)/%.o)


make: $(PROGRAM) 


$(PROGRAM): $(OFILES)
$(BIN_DIR)/%:
	@echo ">>> Linking" $@ "<<<"
	@if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi;
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo ">>> Done <<<"

$(OBJ_DIR)/%.o: %.c
	@echo ">>> Compiling" $< "<<<"
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

# generate depend files and include them to detect changes in header files
$(OBJ_DIR)/%.d: %.c
	@set -e; rm -f $@;
	@echo ">>> Building dependency file for" $< "<<<"
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s@$*.o[ ]*:@$(OBJ_DIR)/$(notdir $@) $(OBJ_DIR)/&@g'\'' > $@'

include $(CFILES:%.c=$(OBJ_DIR)/%.d)

.PHONY: clean install
install:
	ln -sf $(shell pwd)/$(PROGRAM) /usr/bin/$(EXECUTABLE_NAME)

clean:
	rm -rf $(BUILD_DIR)
	rm -f sandbox.log
	rm -f /usr/bin/$(EXECUTABLE_NAME)

