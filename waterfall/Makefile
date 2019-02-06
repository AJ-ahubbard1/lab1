# cs 3350 waterfall
# to compile your project, type make and press enter
CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -lm

all: waterfall

waterfall: waterfall.cpp
	g++ $(CFLAGS) waterfall.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o waterfall 

clean:
	rm -f waterfall
	rm -f *.o

