cc = g++ # clang++

cflags = -I ${srcdir} -std=c++17 -O2
cincludes := $(shell pkg-config --cflags libnsl)

lflags = -flto
llibs := $(shell pkg-config --libs libnsl)

srcdir = src
objdir = obj
bindir = bin


${objdir}/%.o: ${srcdir}/%.cpp
	mkdir -p $(dir $@)
	${cc} ${cflags} ${cincludes} -c $< -o $@


all: clean client server


client: obj/socket/addr.o obj/socket/sock.o obj/socket/connection.o obj/client/main.o
	mkdir -p ${bindir}
	${cc} ${lflags} ${llibs} $+ -o ${bindir}/$@

server: obj/socket/addr.o obj/socket/sock.o obj/socket/server.o obj/socket/connection.o obj/server/main.o
	mkdir -p ${bindir}
	${cc} ${lflags} ${llibs} $+ -o ${bindir}/$@


clean:
	rm -rf ${objdir}
	rm -rf ${bindir}
