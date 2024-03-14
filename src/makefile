# TODO: Make the way I include library files significantly less dumb!

# Variables
SRC_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

INC_DIRS := ../include ../libs/progressbar ../libs/rtklib-2.4.3/src

PROGRESS_DIR := ../libs/progressbar
PROGRESS_FILES := progressbar.c
PROGRESS_FILES := $(addprefix $(PROGRESS_DIR)/,$(PROGRESS_FILES))

RTKLIB_DIR := ../libs/rtklib-2.4.3/src
RTKLIB_FILES := rinex.c rtkcmn.c ephemeris.c preceph.c sbas.c
RTKLIB_FILES := $(addprefix $(RTKLIB_DIR)/,$(RTKLIB_FILES))

BUILD_DIR := ../build
OBJECT_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES)) $(patsubst $(PROGRESS_DIR)/%.c,$(BUILD_DIR)/%.o,$(PROGRESS_FILES)) $(patsubst $(RTKLIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(RTKLIB_FILES)) 

INC_PARAMS := $(foreach d, $(INC_DIRS), -I$(d))
LDFLAGS := -lczmq -lncurses

CFLAGS := -g -std=c99 -Wimplicit-function-declaration -Wall -Wextra -pedantic
RTKLIB_CFLAGS := -g -fpermissive -DENAGLO -DENAGAL -DENAQZS -DENACMP -DENAIRN

# Link the object files
gnss-sim : $(OBJECT_FILES)
	g++ -o $(BUILD_DIR)/$@ $^ $(LINK_PARAMS) $(LDFLAGS)

# Compile all the source files
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	gcc -c -o $@ $< $(INC_PARAMS) $(CFLAGS)

$(BUILD_DIR)/%.o : $(PROGRESS_DIR)/%.c
	gcc -c -o $@ $< $(INC_PARAMS) $(CFLAGS)

$(BUILD_DIR)/%.o : $(RTKLIB_DIR)/%.c
	gcc -c -o $@ $< $(INC_PARAMS) $(RTKLIB_CFLAGS)

# Kick-off the build directory creation
$(OBJECT_FILES): | $(BUILD_DIR)

# Create the build directory
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# Run the compiled program
run :
	./$(BUILD_DIR)/gnss-sim

# Clean-up the build files
clean :
	rm -r $(BUILD_DIR)/