CFLAGS   = -Wall -std=gnu99
INCLUDES = -I./inc
LIBS     = -lm
OBJDIR   = obj
INCDIR = inc

# keep track of these files
KEEP_TRACK = Makefile

SERVER_SRCS = server.c ack_manager.c device.c defines.c utils/err_exit.c utils/shared_memory.c utils/semaphore.c utils/fifo.c
SERVER_OBJS = $(addprefix $(OBJDIR)/, $(SERVER_SRCS:.c=.o))

CLIENT_SRCS = client.c defines.c utils/fifo.c utils/err_exit.c
CLIENT_OBJS = $(addprefix $(OBJDIR)/, $(CLIENT_SRCS:.c=.o))

# must be first rule
default: all

$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/utils

%.h:
	@echo "(missing header $@)"

$(OBJDIR)/%.o: %.c $(INCDIR)/%.h $(KEEP_TRACK)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $@ $<

$(OBJDIR)/utils/%.o: utils/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $@ $<

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ $^
	@echo "Server compiled."
	@echo

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ $^
	@echo "Client compiled."
	@echo

run: clean server
	@./server 100 input/file_posizioni.txt

clean:
	@rm -vf ${SERVER_OBJS} ${CLIENT_OBJS}
	@rm -vf server
	@rm -vf client
	@rm -vf out/*
	@rm -vf /tmp/dev_fifo.*
	@ipcrm -a
	@echo "Removed object files and executables..."

all: $(OBJDIR) server client

.PHONY: run clean
