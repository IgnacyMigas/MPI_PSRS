CXX = mpicc
CXXFLAGS = -lmpe -lm -lpthread
EXEC = PSRS
SRC = utilities.c profiles.c
OBJ = $(SRC:.c=.o)
LIB = -I/opt/nfs/mpe2-2.4.9b/include -L/opt/nfs/mpe2-2.4.9b/lib
N = 10

all: $(EXEC)

$(EXEC): $(EXEC).o $(OBJ)
	$(CXX) $(LIB) $(CXXFLAGS) $(EXEC).o $(OBJ) -o $(EXEC).out   

.c.o:
	$(CXX) $(LIB) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXEC).out

rebuild: | clean
	make $(EXEC)

val:
	valgrind --tool=memcheck --leak-check=full ./$(EXEC).out

.PHONY: clean rebuild

run: $(EXEC)
	/opt/nfs/mpich-3.2/bin/mpiexec -n ${N} ./$(EXEC).out ${FILE}
