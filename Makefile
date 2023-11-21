
#***************************************#
#                                       #
#    Makefile to build MDcat package    #
#                                       #
#***************************************#

PKGNAME  :=  MDcat
ELFNAME  :=  mdcat
TARGET   :=  $(ELFNAME).out

PREFIX   ?= /usr/local
SRCDIR   :=  src
OBJDIR   :=  obj

CC       ?=  gcc
CFLAGS   += -g -O3 -MMD -fPIE

WFLAGS   += -Wall -Wextra -Wstrict-prototypes # -Wpedantic
WFLAGS   += -Walloca -Warray-bounds -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Wformat-security \
            -Wimplicit-fallthrough -Wint-conversion -Wnull-dereference -Wshadow -Wshift-overflow \
            -Wstack-protector -Wstrict-overflow=4 -Wswitch-default -Wswitch-enum -Wundef -Wvla

FSTACK   := -fstack-protector-all -fstack-protector-strong
FASAN    := -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fno-omit-frame-pointer \
            -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow

ifneq (,$(wildcard $(HOME)/.termux))
    CPPFLAGS += -DTERMUX
    WFLAGS   +=
else
    CPPFLAGS += -Wdate-time -D_FORTIFY_SOURCE=2
    WFLAGS   += -Wduplicated-branches -Wduplicated-cond -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 \
                -Wlogical-op -Wstringop-overflow=4 -Wtraditional-conversion -Wtrampolines -Wstack-usage=4096
    FSTACK   += -fstack-clash-protection
    FASAN    += -fsanitize=bounds-strict
endif

CFLAGS  += $(FSTACK) $(FASAN)
LDFLAGS += $(FASAN)

BOLD := $(shell tput bold)
NC   := $(shell tput sgr0)

# MDcat sources
SRCS := $(SRCDIR)/main.c $(SRCDIR)/stack.c

# Object files
OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)

# Build dependencies
DEPS := $(OBJS:%.o=%.d)

# Makefile execution starts here
all: info $(TARGET)

info:
	@echo "\n$(BOLD)Building $(PKGNAME) ...$(NC)"
	@echo "COMPILER : $(CC)"
	@echo "CFLAGS   : $(CFLAGS)"
	@echo "CPPFLAGS : $(CPPFLAGS)"
	@echo "LDFLAGS  : $(LDFLAGS)"

# Link object files and generate executable
$(TARGET): $(OBJS)
	@echo  "\n$(BOLD)Linking object files ...$(NC)"
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "$(BOLD)$(PKGNAME)$(NC) build completed [$(BOLD)$(TARGET)$(NC)]"

# Build object file for each .c file
$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(WFLAGS) $(CPPFLAGS) -c $< -o $@

SYSBINDIR := $(PREFIX)/bin
install:
	install -D $(TARGET) $(DESTDIR)$(SYSBINDIR)/$(ELFNAME)

uninstall:
	rm -f $(DESTDIR)$(SYSBINDIR)/$(ELFNAME)

KTAGSDIR  := __ktags
COREFILES := core vgcore*
clean:
	rm -rf $(TARGET) $(OBJDIR) $(COREFILES) $(KTAGSDIR)

-include $(DEPS)

.PHONY: all clean info install uninstall

#EOF
