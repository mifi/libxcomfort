CFLAGS = -Wall -g
LIBS = -lusb-1.0

default: testclient xcomfortd

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

testclient: testclient.o libxcomfort.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

xcomfortd: xcomfortd.o libxcomfort.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lmicrohttpd -o $@

mqtt_xcomfort: mqtt_xcomfort.o libxcomfort.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lpaho-mqtt3cs -ljansson -o $@

clean:
	rm -rf testclient xcomfortd mqtt_xcomfort *.o
