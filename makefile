CFLAGS := -Wall -Werror -std=gnu2x -pedantic
LDFLAGS := -lm

BIN := bin

OBJDIR := objs

SRC := $(wildcard *.c platform/*.c)
OBJ := $(SRC:%.c=$(OBJDIR)/%.o)

.PHONY = clean

all: cutils.a cutils.so

cutils.a: $(OBJ)
	ar rcs $(BIN)/$@ $(OBJ)

cutils.so: cutils.a
	$(CC) -shared $(CFLAGS) $(BIN)/$< $(LDFLAGS) -o $(BIN)/$@

clean:
	rm -f $(OBJ) $(BIN)/cutils.a $(BIN)/cutils.so

dirs:
	mkdir $(BIN) $(OBJDIR) $(OBJDIR)/platform

$(OBJDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<