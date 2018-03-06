# Program wide settings
EXE       := basic-cc
EXEC      := BASICCC
BASICCC_VERSION   := 0
BASICCC_SUBLEVEL  := 1
BASICCC_PATCH     := 0
BASICCC_VERSION_N := $(BASICCC_VERSION).$(BASICCC_SUBLEVEL).$(BASICCC_PATCH)

BASICCC_LIBFLAGS :=
BASICCC_CFLAGS += -I'./include'                             \
				   -Wall -Wextra -Wno-unused-parameter      \
                   -DBASICCC_VERSION=$(BASICCC_VERSION)       \
                   -DBASICCC_SUBLEVEL=$(BASICCC_SUBLEVEL)     \
                   -DBASICCC_PATCH=$(BASICCC_PATCH)           \
                   -DBASICCC_VERSION_N="$(BASICCC_VERSION_N)"

BASICCC_OBJS += ./src.o

