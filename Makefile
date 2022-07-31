CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror

PROMPT = -DPROMPT

EXECS = 33sh 33noprompt
DEPENDENCIES = sh.c

.PHONY: all clean

all: $(EXECS)

33sh: $(DEPENDENCIES)
	gcc $(CFLAGS) $^ $(PROMPT) -o $@

33noprompt: $(DEPENDENCIES)
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECS)
