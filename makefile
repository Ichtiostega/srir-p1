GXX=/opt/nfs/mpich-3.2/bin/mpicxx
FLAGS=-DSPRNG_MPI -DUSE_MPI -I/opt/nfs/sprng5/include -L/opt/nfs/sprng5/lib -lsprng 
EXEC=mc_temperature_propagation

$(EXEC): $(EXEC).cpp
	$(GXX) -o $@ $< $(FLAGS) 

run: nodes $(EXEC)
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC)

test: nodes $(EXEC)
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC) 0.01 10 10
	@echo "Expected value: ~25"
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC) 0.01 10 1
	@echo "Expected value: ~90"
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC) 0.01 10 19
	@echo "Expected value: ~1.73"
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC) 0.01 19 19
	@echo "Expected value: ~0.27"
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC) 0.01 1 1
	@echo "Expected value: ~50"
nodes:
	/opt/nfs/config/station_name_list.sh 101 116 > nodes

clean:
	rm $(EXEC) nodes
