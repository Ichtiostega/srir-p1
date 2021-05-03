

mc_temperature_propagation: mc_temperature_propagation.cpp
	/opt/nfs/mpich-3.2/bin/mpicxx -DSPRNG_MPI -DUSE_MPI -o $@ $< -I/opt/nfs/sprng5/include -L/opt/nfs/sprng5/lib -lsprng 

run: nodes
	mpiexec -f nodes -n $$(( 2 * $$(cat nodes | wc -l) )) ./mc_temperature_propagation

nodes:
	/opt/nfs/config/station_name_list.sh 101 116 > nodes

clean:
	rm mc_temperature_propagation nodes
