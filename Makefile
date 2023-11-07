DEBUG      ?= 1
UNAME := $(uname)


CC         ?= gcc
CFLAGS     ?= -std=gnu17 -Wall -Wextra -Wpedantic \
              -Wformat=2 -Wno-unused-parameter -Wshadow \
              -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
              -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
              -Wno-format-nonliteral # -Wno-incompatible-pointer-types-discards-qualifiers 
ifeq ($(CC),gcc)
  CFLAGS   += -Wjump-misses-init -Wlogical-op
endif

CFLAGS_RELEASE = $(CFLAGS) -O3 -DNDEBUG -DNTRACE
ifeq ($(CC),gcc)
  CFLAGS_RELEASE += -flto
endif

CFLAGS_DEBUG   = $(CFLAGS) -O0 -g3 -fstack-protector -ftrapv -fwrapv
CFLAGS_DEBUG  += -fsanitize=address,undefined

PREFIX     ?= /usr/local
BINDIR     ?= $(PREFIX)/bin
MANDIR     ?= $(PREFIX)/man/man1
INSTALL    ?= install -s

SRCDIR     ?= src
OBJDIR     ?= obj

PROG       = ccc
MAN        = $(PROG).1

CFILES     != ls $(SRCDIR)/*.c
COBJS       = ${CFILES:.c=.o}
COBJS      := $(subst $(SRCDIR), $(OBJDIR), $(COBJS))

ifeq ($(DEBUG),1)
	_CFLAGS := $(CFLAGS_DEBUG)
else
	_CFLAGS := $(CFLAGS_RELEASE)
endif

all: prepare $(PROG)
prepare: $(OBJDIR)

$(PROG): $(COBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(_CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) $(_CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

install: all
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)
	$(INSTALL) $(PROG) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 $(MAN) $(DESTDIR)$(MANDIR)/$(MAN)

clean:
	rm -rf $(PROG) $(OBJDIR)

.PHONY: all install clean
