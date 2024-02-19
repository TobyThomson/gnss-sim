# Variables
SRC_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

INC_DIRS := ../include ../libs/rtklib-2.4.3/src
RTKLIB_DIR := ../libs/rtklib-2.4.3/src
RTKLIB_FILES := rtkcmn.c
RTKLIB_FILES := $(addprefix $(RTKLIB_DIR)/,$(RTKLIB_FILES))

BUILD_DIR := ../build
OBJECT_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES)) $(patsubst $(RTKLIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(RTKLIB_FILES))

INC_PARAMS := $(foreach d, $(INC_DIRS), -I$(d))
LDFLAGS := -lczmq

CFLAGS := -fpermissive

# Link the object files
gnss-sim : $(OBJECT_FILES)
	g++ -o $(BUILD_DIR)/$@ $^ $(LINK_PARAMS) $(LDFLAGS)

# Compile all the source files
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	g++ -c -o $@ $< $(INC_PARAMS) $(CFLAGS)

$(BUILD_DIR)/%.o : $(RTKLIB_FILES)
	g++ -c -o $@ $< $(INC_PARAMS) $(CFLAGS)

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