CXX = xmpcc
CXXFLAGS =
EXEC = PSRS_xmp
SRC = utilities.c
OBJ = $(SRC:.c=.o)
LIB =
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

.PHONY: clean rebuild

run:
ifdef NODES
	$(eval ARGS1=-f $(NODES))
endif
ifdef FILE
	$(eval ARGS2=-f $(FILE))
endif
	/opt/nfs/mpich-3.2/bin/mpiexec -n ${N} ${ARGS1} ./$(EXEC).out $(ARGS2)

