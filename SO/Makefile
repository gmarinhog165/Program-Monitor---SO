CFLAGS = -g -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -Iincludes
#-o2 -g

all: folders server client

server: bin/monitor

client: bin/tracer

folders:
	@mkdir -p src obj bin tmp

bin/monitor: obj/monitor.o
	gcc ${CFLAGS} obj/monitor.o -o bin/monitor -lm -lglib-2.0

obj/monitor.o: src/monitor.c
	gcc -Wall ${CFLAGS} -c src/monitor.c -o obj/monitor.o

bin/tracer: obj/tracer.o
	gcc ${CFLAGS} obj/tracer.o -o bin/tracer

obj/tracer.o: src/tracer.c
	gcc -Wall ${CFLAGS} -c src/tracer.c -o obj/tracer.o

clean:
	rm -f obj/* tmp/* bin/{tracer,monitor}