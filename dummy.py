import sys


for i in range(int(3e6)):
    a = 1 + 1

string = sys.stdin.readline()
print(f'{string.rstrip()} {sys.argv[1]}')
print(f'{string.rstrip()} {sys.argv[1]}', file=sys.stderr)
