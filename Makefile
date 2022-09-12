#de bestanden die gecompiled moeten worden
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

#de compiler die gebruikt moet worden
CC = g++

#de bestanden van de librarys die geinclude moeten worden
INCLUDE_PATHS = -Iinclude
#de locar=tie van de library bestanden
LIBRARY_PATHS = -Llib
#flags die de compiler gebruikt (kan bijv. optimalisatie met -O2)
COMPILER_FLAGS = -std=c++20 -static-libstdc++ -static-libgcc -O2 -pthread
#de library bestanden die gebruikt moeten worden 
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net
#de locatie van het output bestand
OBJ_NAME = build/rc-car-controller

#het compile command samengevoegd
all:${OBJ}
	${CC} ${OBJ} ${COMPILER_FLAGS} ${INCLUDE_PATHS} ${LIBRARY_PATHS} ${LINKER_FLAGS}  -o ${OBJ_NAME}

.cpp.o:
	${CC} ${COMPILER_FLAGS} ${INCLUDE_PATHS} -c $*.cpp -o $@

clean:
	-rm -f src/*.o