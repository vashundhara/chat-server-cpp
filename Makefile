all: server client

server: server.cpp
	clang++ server.cpp -o server -lpthread

client: client.cpp
	clang++ client.cpp -o client -lpthread

clean:
	rm -f server client

