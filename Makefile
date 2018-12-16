all: run

timekeeper: timekeeper.c
	gcc $^ -o $@ -w

run: timekeeper
	./$^ python3 dummy.py

clean:
	rm timekeeper -rf

.PHONY: run
