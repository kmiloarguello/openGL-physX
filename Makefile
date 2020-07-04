CC= g++
RM=rm -f
LINKER_FLAGS= -lSDL2 -lGL -lGLEW
COMPILER_FLAGS= -O3 -w
OBJ_NAME=main

SRCS= $(wildcard ./*.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

$(OBJ_NAME):$(OBJS)
        $(CC) $(COMPILER_FLAGS) -o $(OBJ_NAME) $(OBJS) $(LINKER_FLAGS)

%.o: %.cpp
        $(CC) $(COMPILER_FLAGS) -c $<

.depend: $(SRCS)
        $(RM) ./.depend
        $(CC) $(COMPILER_FLAGS) -MM $$^>>./.depend

clean:
        $(RM) *.o $(OBJ_NAME)
