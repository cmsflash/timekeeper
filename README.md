# timekeeper

timekeeper is an elementary shell.
It accepts a set of commands, executes them, and prints out usage statistics.

## Execution and statistics
An example usage is
``` sh
./timekeeper echo Hello, World
```

With likely output
```
Process with id: 86159 created for the command: echo
Hello World
The command "echo" terminated with returned status code = 0
real: 0.00 s, user: 0.00 s, system: 0.00 s, context switch: 0
```
where real, user, system, and context switch stands for the wall-clock time, user time, system time, and number of context switches, respectively.

## Pipe support

timekeeper supports pipes.
To be noted, the default pipe symbol is `!`, instead of `|`.

An example usage:
``` sh
./timekeeper echo Hello ! python3 dummy.py World
```

With likely output
```
Process with id: 2771 created for the command: echo
Process with id: 2772 created for the command: python3
Hello World
The command "echo" terminated with returned status code = 0
real: 0.00 s, user: 0.00 s, system: 0.00 s, context switch: 0
The command "echo" terminated with returned status code = 0
real: 0.22 s, user: 0.00 s, system: 0.00 s, context switch: 0
```
