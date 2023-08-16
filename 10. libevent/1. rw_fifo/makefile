src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src))

ALL: $(target)

%: %.c
	gcc $< -o $@ -Wall -g -levent

clean:
	-rm -rf $(target)

.PHONY: clean ALL

