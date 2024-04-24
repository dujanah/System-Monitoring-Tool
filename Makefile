CC = gcc
CFLAGS = -Wall -Werror -g
EXE = smt
HEADER = stats_functions.h

# define the .PHONY target names that are not files
.PHONY: all clean help

all: $(EXE)

## $(EXE): build executable from object files
$(EXE): main.o stats_functions.o
	$(CC) $(CFLAGS) $^ -o $@

## %.o: compile source files to object files; dependent on header files
%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

## clean: remove generated object and executable files
clean:
	rm -f *.o $(EXE)

## help: more information on targets
help: Makefile
	@sed -n 's/^##//p' $<