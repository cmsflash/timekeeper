all: run

timekeeper: timekeeper.c
	gcc $^ -o $@ -w

run: timekeeper
	./$^ python3 dummy.py ! echo

clean:
	rm timekeeper -rf

.PHONY: run
