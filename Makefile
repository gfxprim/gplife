CFLAGS?=-W -Wall -Wextra -O2
CFLAGS+=$(shell gfxprim-config --cflags)
LDLIBS=-lgfxprim $(shell gfxprim-config --libs-widgets)
BIN=gplife
DEP=$(BIN:=.dep)

all: $(DEP) $(BIN)

$(BIN): board.o

%.dep: %.c
	$(CC) $(CFLAGS) -M $< -o $@

-include $(DEP)

install:
	install -m 644 -D layout.json $(DESTDIR)/etc/gp_apps/$(BIN)/layout.json
	install -D $(BIN) -t $(DESTDIR)/usr/bin/
	install -d $(DESTDIR)/usr/share/applications/
	install -m 644 $(BIN).desktop -t $(DESTDIR)/usr/share/applications/
	install -d $(DESTDIR)/usr/share/$(BIN)/
	install -m 644 $(BIN).png -t $(DESTDIR)/usr/share/$(BIN)/

clean:
	rm -f $(BIN) *.dep *.o
