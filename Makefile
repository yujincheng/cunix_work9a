CFILES = $(wildcard *.c)
TARGETS = $(CFILES:.c=)
cc ?= clang
CFLAGS = -Wall -DDEBUG -g3 -lreadline

all: $(TARGETS)

$(TARGETS): $(CFILES)
	$(CC) $(CFLAGS) -o $@ $(@:=.c)

clean: $(TARGETS)
	rm $(TARGETS)
