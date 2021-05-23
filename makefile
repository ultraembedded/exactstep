###############################################################################
## Simulator Makefile
###############################################################################

# Target
TARGET	   ?= exactstep

HAS_SCREEN ?= False

# Source Files
SRC_DIR    = core peripherals cpu-rv32 cpu-rv64 cpu-armv6m cpu-mips-i cli platforms device-tree display net virtio

CFLAGS	    = -O2 -fPIC
CFLAGS     += -Wno-format
CFLAGS     += -DINCLUDE_NET_DEVICE
ifneq ($(HAS_SCREEN),False)
  CFLAGS   += -DINCLUDE_SCREEN
endif

INCLUDE_PATH += $(SRC_DIR)
CFLAGS       += $(patsubst %,-I%,$(INCLUDE_PATH))

LDFLAGS     = 
LIBS        = -lelf -lbfd -lfdt

ifneq ($(HAS_SCREEN),False)
  LIBS     += -lSDL
endif

###############################################################################
# Variables
###############################################################################
OBJ_DIR      ?= obj/

###############################################################################
# Variables: Lists of objects, source and deps
###############################################################################
# SRC / Object list
src2obj       = $(OBJ_DIR)$(patsubst %$(suffix $(1)),%.o,$(notdir $(1)))

SRC          ?= $(foreach src,$(SRC_DIR),$(wildcard $(src)/*.cpp))
OBJ          ?= $(foreach src,$(SRC),$(call src2obj,$(src)))
LIB_OBJ      ?= $(foreach src,$(filter-out main.cpp,$(SRC)),$(call src2obj,$(src)))

###############################################################################
# Rules: Compilation macro
###############################################################################
define template_cpp
$(call src2obj,$(1)): $(1) | $(OBJ_DIR)
	@echo "# Compiling $(notdir $(1))"
	@g++ $(CFLAGS) -c $$< -o $$@
endef

###############################################################################
# Rules
###############################################################################
all: $(TARGET) 
	
$(OBJ_DIR):
	@mkdir -p $@

$(foreach src,$(SRC),$(eval $(call template_cpp,$(src))))	

$(TARGET): $(OBJ) makefile
	@echo "# Linking $(notdir $@)"
	@g++ $(LDFLAGS) $(OBJ) $(LIBS) -o $@

clean:
	-rm -rf $(OBJ_DIR) $(TARGET)

