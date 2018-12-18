all: run

timekeeper: timekeeper.c
	gcc $^ -o $@ -w

run: timekeeper
	./$^ echo Success ! python3 dummy.py 1 ! python3 dummy.py 2 ! python3 dummy.py 3 ! python3 dummy.py 4

clean:
	rm timekeeper -rf

.PHONY: run

