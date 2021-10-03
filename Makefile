PROJDIR := ${CURDIR}

target := mkc
target_lib := libmonkeycall.so

BISON_SRC += monkey/core/syntax/_parser.c

LIB_SRC :=
LIB_SRC += $(wildcard monkey/core/*.c)
LIB_SRC += $(wildcard monkey/libs/*.c)
LIB_SRC += $(wildcard monkey/core/syntax/*.c)
LIB_SRC += $(wildcard monkey/core/semantics/*.c)
LIB_SRC += $(wildcard monkey/core/execution/*.c)
LIB_SRC += $(wildcard monkey/build-in-cbs/*.c)
LIB_SRC += $(BISON_SRC)
LIB_OBJS += $(LIB_SRC:%.c=%.o)

C_SRC := $(LIB_SRC)
C_SRC += $(wildcard *.c)
C_OBJS := $(C_SRC:%.c=%.o)
C_GCNO := $(C_SRC:%.c=%.gcno)

CFLAGS := -Wall -Werror -fPIC

ifeq ($(shell uname), NetBSD)
LDFLAGS := -lpthread -lcrypto
else ifeq ($(shell uname), OpenBSD)
LDFLAGS := -lpthread -lcrypto
else
LDFLAGS := -lpthread -ldl -lcrypto
endif

CCINCLUDES := -I$(PROJDIR) -I$(PROJDIR)/monkey

ifeq ($(shell uname), FreeBSD)
LDFLAGS += -lexecinfo
else ifeq ($(shell uname), NetBSD)
LDFLAGS += -lexecinfo
else ifeq ($(shell uname), OpenBSD)
LDFLAGS += -lexecinfo
endif

INSTALL_LIB_PATH := /usr/lib
INSTALL_BIN_PATH := /usr/bin
INSTALL_HEADER_PATH := /usr/include

ifdef profile
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGS += -fprofile-arcs -ftest-coverage -lgcov
endif

ifdef tc_enumerator
CFLAGS += -DTC_ENUMERATOR=
endif

ifdef debug
CFLAGS += -ggdb3 -O0 -DSTATIC= -Ddebug=1
BISONOPT := -rall
else
CFLAGS += -O2 -DSTATIC=static
BISONOPT :=
endif

GCC ?= clang
BISON := bison
RM := rm

%.o:%.c
	@$(GCC) $(CFLAGS) $(CCINCLUDES) -c $< -o $@

all: depend $(target) $(target_lib)

depend: $(C_SRC)
	@$(GCC) $(CFLAGS) $(CCINCLUDES) -MM $^ > $@

-include depend

$(target): $(C_OBJS)
	@$(GCC) -Wl,-E $^ -o $@ $(LDFLAGS)

$(target_lib): $(LIB_OBJS)
	@$(GCC) -shared $^ -o $@ $(LDFLAGS)

monkey/core/syntax/_parser.o: monkey/core/syntax/_parser.c
	@$(GCC) $(CFLAGS) $(CCINCLUDES) -c $< -o $@

monkey/core/syntax/_parser.c: monkey/core/syntax/parser.y
	@$(BISON) -d $< -o $@ $(BISONOPT)

test: $(target)
	@./sct.sh

clean: force
	-@$(RM) -f depend $(target) $(target_lib) $(C_OBJS) $(C_GCNO)
	-@$(RM) -f monkey/core/syntax/_parser.h monkey/core/syntax/_parser.c
	-@$(RM) -f monkey/core/syntax/_parser.output monkey/core/syntax/_parser.o

install: $(target_lib) $(target)
	@cp ${target_lib} ${INSTALL_LIB_PATH}
	@cp ${target} ${INSTALL_BIN_PATH}
	@cp monkeycall.h ${INSTALL_HEADER_PATH}

uninstall: force
	@rm ${INSTALL_LIB_PATH}/${target_lib}
	@rm ${INSTALL_BIN_PATH}/${target}
	@rm ${INSTALL_HEADER_PATH}/monkeycall.h

force:

# eof
