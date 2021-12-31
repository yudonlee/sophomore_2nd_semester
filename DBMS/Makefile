.SUFFIXES: .c .o

CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.c)
# OBJS:=$(SRCS:.c=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.c
TARGET_OBJ:=$(SRCDIR)main.o
STATIC_LIB:=$(LIBS)libbpt.a
DYNAMIC_LIB:=$(LIBS)libbpt.so

# Include more files if you write another source file.
SRCS_FOR_LIB:= \
	$(SRCDIR)univ.c \
	$(SRCDIR)global_vars.c \
	$(SRCDIR)delete.c \
	$(SRCDIR)insert.c \
	$(SRCDIR)bpt_ext.c \
	$(SRCDIR)db.c \
	$(SRCDIR)file.c

OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)

CFLAGS+= -g -fPIC -I $(INC)

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ) $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -o $@ -L $(LIBS) -lbpt

%.o:	%.c
	$(CC) $(CFLAGS) $^ -c -o $@

clean:
	rm -rf $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(LIBS)*

$(DYNAMIC_LIB):	$(OBJS_FOR_LIB)
	gcc -shared -Wl,-soname,libbpt.so -o $@ $(OBJS_FOR_LIB)

$(STATIC_LIB):	$(OBJS_FOR_LIB)
	ar cr $@ $^

