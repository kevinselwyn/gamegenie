NAME    := gamegenie
FLAGS   := -Wall -Wextra -fdata-sections -ffunction-sections
BIN_DIR := /usr/local/bin

all: build

build: $(NAME).c
	gcc $(FLAGS) -o $(NAME) $< -O2

test: build
	./$(NAME) -d "GOSSIP"
	./$(NAME) -e 0xD1DD 0x14
	./$(NAME) -d "ZEXPYGLA"
	./$(NAME) -e 0x94A7 0x02 0x03

install: $(NAME)
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

clean:
	rm -f $(NAME)