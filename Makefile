CFLAGS   = -Wall -std=gnu99
INCLUDES = -I .
OBJDIR   = obj

SERVER_SRCS = server.c ack_manager.c device.c defines.c utils/err_exit.c utils/shared_memory.c utils/semaphore.c utils/fifo.c
SERVER_OBJS = $(addprefix $(OBJDIR)/, $(SERVER_SRCS:.c=.o))

CLIENT_SRCS = client.c defines.c utils/fifo.c utils/err_exit.c
CLIENT_OBJS = $(addprefix $(OBJDIR)/, $(CLIENT_SRCS:.c=.o))

all: $(OBJDIR) server client

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

client: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)/utils

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJDIR)/utils/%.o: utils/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

run: clean server
	@./server 100 input/file_posizioni.txt

clean:
	@rm -vf ${SERVER_OBJS}
	@rm -vf ${CLIENT_OBJS}
	@rm -vf server
	@rm -vf client
	@rm -vf /tmp/dev_fifo.*
	@ipcrm -a
	@echo "Removed object files and executables..."

.PHONY: run clean
