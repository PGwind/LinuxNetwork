src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src))

ALL: $(target)

%: %.c
	gcc $< -o $@ -Wall -g -I./inc -lwrap -lpthread

clean:
	-rm -rf $(target)

.PHONY: clean ALL
