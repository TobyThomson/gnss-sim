# Variables
SRC_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

BUILD_DIR := ../build
OBJECT_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INC_DIR := ../include
LIBS_DIR := ../libs
LIBS := 

INC_PARAMS := $(foreach d, $(LIBS), -I$(LIBS_DIR)/$d/include) $(foreach d, $(INC_DIR), -I$(INC_DIR))
LINK_PARAMS := $(foreach d, $(LIBS), -L$(LIBS_DIR)/$d/lib)

LDFLAGS := -lczmq

# Link the object files
gnss-sim : $(OBJECT_FILES)
	g++ -o $(BUILD_DIR)/$@ $^ $(LINK_PARAMS) $(LDFLAGS)

# Compile all the source files
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	g++ -c -o $@ $< $(INC_PARAMS)

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