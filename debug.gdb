target remote tcp:localhost:1234
symbol-file build/kernel
b src/main.c:177
c