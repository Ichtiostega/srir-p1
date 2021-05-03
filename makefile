GXX=/opt/nfs/mpich-3.2/bin/mpicxx
FLAGS=-DSPRNG_MPI -DUSE_MPI -I/opt/nfs/sprng5/include -L/opt/nfs/sprng5/lib -lsprng 
EXEC=mc_temperature_propagation

$(EXEC): $(EXEC).cpp
	$(GXX) -o $@ $< $(FLAGS) 

run: nodes $(EXEC)
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./$(EXEC)

nodes:
	/opt/nfs/config/station_name_list.sh 101 116 > nodes

clean:
	rm $(EXEC) nodes
