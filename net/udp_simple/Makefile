VERSION =	1.3

#INCLUDE =	-I/usr/devel/include/Sockets \
#		-I/usr/local/include/Sockets

#CFLAGS =	-Wall -g -O2 $(INCLUDE) -MD `Sockets-config`

INCLUDE = 
CFLAGS = 
CPPFLAGS =	$(CFLAGS)

LIBS =
#LIBS =		-L/usr/local/lib -lSockets -lpthread -lssl -lcrypto


PROGS =		udpserver udpclient
all:		$(PROGS)

dig:		dig.o
		gcc -o $@ $^ $(LIBS)

udpserver:	udpserver.c
		gcc -o $@ $^ $(LIBS)
		
udpclient:	udpclient.c
		gcc -o $@ $^ $(LIBS)
		
clean:
		rm -f $(PROGS)


-include	*.d

DIR =		/usr/local/apache/www.libsockets.net/htdocs/project/example9

tar:
		@mkdir -p $(DIR)
		tar czf $(DIR)/udpexample-$(VERSION).tar.gz \
			udpclient.cpp \
			udpserver.cpp \
			UdpTestSocket.h UdpTestSocket.cpp \
			Makefile
		cp UdpTestSocket.h UdpTestSocket.cpp udpserver.cpp udpclient.cpp \
			/usr/local/apache/www.libsockets.net/htdocs/tutorial


