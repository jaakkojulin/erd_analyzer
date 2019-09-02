CC=gcc
ldflags=
libs=-lm
INSTALL=install -c
prefix=/usr/local
bindir=$(prefix)/bin
datadir=$(prefix)/data


CFLAGS=-g -Wall -I. -DDATADIR=$(datadir)

#CFLAGS += -DDEBUG


objects=rgbcolor.o elements.o plot.o depthfile.o depthprofile.o erd_analyzer.o round.o

default: erd_analyzer

clean:
	$(RM) *.o
	$(RM) erd_analyzer

install:
	$(INSTALL) -d $(bindir)
	$(INSTALL) -d $(datadir)
	$(INSTALL) erd_analyzer $(bindir)
	$(INSTALL) colors.txt $(datadir)
	$(INSTALL) elements.txt $(datadir)

$(objects): %.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $< -o $@

erd_analyzer: $(objects)
	$(CC) $(CFLAGS) -o erd_analyzer $(objects) $(ldflags) $(libs)
