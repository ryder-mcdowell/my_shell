all: my_shell

my_shell: my_shell.o
	g++ -o my_shell my_shell.o

my_shell.o: my_shell.cpp
	g++ -c my_shell.cpp

clean:
	rm -f my_shell.o
