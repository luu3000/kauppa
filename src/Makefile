CXX = g++
CC = gcc
CFLAGS += -c -std=c99 -g -Wall -Wextra -pedantic
LDFLAGS = -lm
EXECNAME = main

C_FILES = project.c
COBJECTS = project.o

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	EXECUTABLE = $(EXECNAME).exe
else
	RM=rm -f
	EXECUTABLE = $(EXECNAME)
endif

.PHONY: all

all: main run

main: $(COBJECTS)
	@echo "Linking for executable" $(EXECUTABLE)
	$(CC) $(COBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

$(COBJECTS): %.o:%.c
	@echo "Compiling" $(C_FILES)
	$(CC) $(CFLAGS) $< -o $@
clean:
	@echo "Cleaning" $(C_FILES)
	$(RM) *.o $(EXECUTABLE) $(EXECNAME).exe

run:
	@echo "Running \"$(EXECUTABLE)\""
	@./$(EXECUTABLE)