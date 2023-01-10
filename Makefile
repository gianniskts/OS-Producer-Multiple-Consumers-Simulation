ALL: parent

parent: parent.o utils.o child.o
	g++ parent.o utils.o child.o -o parent -lpthread

parent.o: parent.cpp
	g++ -g -Wall -c parent.cpp

utils.o: utils.cpp
	g++ -g -Wall -c utils.cpp

child.o: child.cpp
	g++ -g -Wall -c child.cpp

clean:
	rm -f parent parent.o child.o

run: ./parent 
	./parent $(ARGS)
