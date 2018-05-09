CXX = /opt/nfs/mpich-3.2/bin/mpicc
CXXFLAGS = -std=c11 -lmpe -lm -lpthread
EXEC = PSRS
SRC = utilities.c profiles.c
OBJ = $(SRC:.c=.o)
LIB = -I/opt/nfs/mpe2-2.4.9b/include -L/opt/nfs/mpe2-2.4.9b/lib
N = 4

all: $(EXEC)

$(EXEC): $(EXEC).o $(OBJ)
	$(CXX) $(EXEC).o $(OBJ) -o $(EXEC).out $(LIB) $(CXXFLAGS)

.c.o:
	$(CXX) $(LIB) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXEC).out

rebuild: | clean
	make $(EXEC)

val:
	valgrind --tool=memcheck --leak-check=full ./$(EXEC).out

.PHONY: clean rebuild

run:
ifdef NODES
	$(eval ARGS1=-f $(NODES))
endif
ifdef FILE
	$(eval ARGS2=-f $(FILE))
endif
	/opt/nfs/mpich-3.2/bin/mpiexec -n ${N} ${ARGS1} ./$(EXEC).out $(ARGS2)

