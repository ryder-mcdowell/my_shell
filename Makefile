all: my_shell

my_shell: my_shell.o main.o
	gcc -o my_shell my_shell.o main.o

my_shell.o: my_shell.cpp my_shell.h
	gcc -c my_shell.cpp

main.o: main.cpp
	gcc -c main.cpp

clean:
	rm -f my_shell.o main.o
