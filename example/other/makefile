.SUFFIXES: .cpp .o

CC=g++

SRCS=client.cpp

			
OBJS=$(SRCS:.cpp=.o)
EXEC=client

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) -lpthread
	@echo '----------------ok------------------'
	
.cpp.o:
	$(CC) -std=c++0x -Wall -g -o $@ -c $< -I/usr/include -I/usr/include/c++/4.8.2/tr1/tuple -I../base/ -I../net/ -I../net/poller/ -I../net/http/ -I../net/inspect/
	
clean:
	rm -f $(OBJS)
	rm -f core*