
# linux
CC=gcc
EXECUTABLE=thothtest
CFLAGS =  -g -Wall -lm  $(shell sdl2-config --cflags) $(shell pkg-config --cflags freetype2)
# CFLAGS =  -g -Wall -lm  -DLINUX_INSTALL -DLINUX_COMPILE $(shell sdl2-config --cflags) $(shell pkg-config --cflags freetype2)
FREETYPELIBS = $(shell pkg-config --libs freetype2)
GLEWLIBS = $(shell pkg-config --static --libs glew)
SDLLIBS = $(shell pkg-config --libs sdl2)
# add -pg for gdb
LDLIBS =-lm -lutil -static-libgcc -L. -lthoth $(GLEWLIBS) $(SDLLIBS) $(FREETYPELIBS) 
# #
# windows
#  CC=i686-w64-mingw32-gcc
#  EXECUTABLE=thoth.exe
#  CFLAGS = -w -Wl,-subsystem,windows -w -fpermissive -Wl,--no-undefined -g -DWINDOWS_COMPILE -I/usr/i686-w64-mingw32/include/freetype2/ -DGLEW_STATIC -Dmain=SDL_main
#  LDLIBS = -L/usr/i686-w64-mingw32/lib/ -lmingw32 -lSDL2main -lSDL2 -mwindows \
#   -lm -lcomctl32 -lwinmm -lmingw32 \
#   -lfreetype -mwindows -lglu32 -lglew32 -lopengl32 -L/usr/i686-w64-mingw32/lib/libglew32.dll.a \
#   -L/usr/i686-w64-mingw32/lib/libglew32.a -lpng -lz


SOURCES=main.c window.c log.c

OBJECTS=$(SOURCES:.c=.o)


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDLIBS) -o $@

# wihndows unused right now
# all: createResourcesO $(SOURCES) $(EXECUTABLE)

# $(EXECUTABLE): $(OBJECTS) icon.o
# 	$(CC) $(OBJECTS) icon.o $(LDLIBS) -o $@

# createResourcesO: icon.rc
# 	i686-w64-mingw32-windres icon.rc -o icon.o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o
