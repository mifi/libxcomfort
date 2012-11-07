CFLAGS = -Wall -g
LIBS = -lusb-1.0

default: testclient

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

testclient: testclient.o libxcomfort.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf testclient *.o

