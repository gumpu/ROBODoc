CFLAGS = -ansi -pedantic -O2 -Wall -I lua -I lpeg
LFLAGS = -ldl -lm

LUA_OBJECTS = lua/lapi.o lua/lauxlib.o lua/lbaselib.o lua/lbitlib.o \
	lua/lcode.o lua/lcorolib.o lua/lctype.o lua/ldblib.o \
	lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o \
	lua/liolib.o lua/llex.o lua/lmathlib.o lua/lmem.o \
	lua/loadlib.o lua/lobject.o lua/lopcodes.o lua/loslib.o lua/lparser.o \
	lua/lstate.o lua/lstring.o lua/lstrlib.o lua/ltable.o lua/ltablib.o \
	lua/ltm.o lua/lundump.o \
	lua/lvm.o lua/lzio.o

robodoc : robodoc.o linit.o
	gcc lpeg/lpeg.o $(LUA_OBJECTS) linit.o robodoc.o -o robodoc $(LFLAGS)

linit.o : linit.c
	gcc $(CFLAGS) -DLUA_COMPAT_ALL -DLUA_USE_LINUX -c linit.c -o linit.o

robodoc.o : robodoc.c
	gcc $(CFLAGS) -c robodoc.c -o robodoc.o

clean :
	-rm robodoc.o robodoc linit.o

