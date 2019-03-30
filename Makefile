CC=gcc
CC_FLAGS=-g -Wall
main: main.o mylist.o mydir.o mysearch.o
	$(CC) $(CC_FLAGS) -o $@ $^
main.o: main.c
	$(CC) $(CC_FLAGS) -c -o $@ $^
mylist.o: mylist.c
	$(CC) $(CC_FLAGS) -c -o $@ $^
mydir.o: mydir.c
	$(CC) $(CC_FLAGS) -c -o $@ $^
mysearch.o: mysearch.c
	$(CC) $(CC_FLAGS) -c -o $@ $^
install: main
	cp main /usr/local/bin/simfind
clean:
	rm -rf main.o mylist.o mydir.o mysearch.o main
