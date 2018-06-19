CXX = xmpcc
CXXFLAGS = 
EXEC = PSRS
SRC = utilities.c
OBJ = $(SRC:.c=.o)
LIB = -I/opt/nfs/cuda/include -L/opt/nfs/cuda/lib64
N = 4

all: $(EXEC)

$(EXEC): $(EXEC).o $(OBJ)
	$(CXX) $(EXEC).o $(OBJ) -o $(EXEC).out $(LIB) $(CXXFLAGS) -lcudart

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
	/opt/nfs/mpich-3.2/bin/mpiexec -n ${N} ${ARGS1} -errfile-pattern /dev/null ./$(EXEC).out $(ARGS2) | egrep -v '(context|handle)'

