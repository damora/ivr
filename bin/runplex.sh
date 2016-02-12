#!/bin/bash
#export OMP_NUM_THREADS=2
#export REG_RELAY_ADDRESS=172.16.5.1:1300
#export REG_CONNECTOR_ADDRESS=172.16.5.1:21370
#export REG_RELAY_ADDRESS=172.16.5.101:1300
#export REG_RELAY_ADDRESS=localhost:1300
#export BG_COREDUMPONEXIT=1


#rm *.raw
#rm *.tiff

# use slrun.sh for BGQ
# debug mpirun for Mac
#mpirun -np 1  ./ivrserver 2>&1 | tee ivrserver.out

# mpirun for Mac
#mpirun -np 1 gdb  ./ivrserver  : -np 7 ./ivrserver 2>&1 | tee ivrserver.out
mpirun  -np 2 ./ivrserver_linux ~damora/Data/Shell/vel_grid_small.bin 501 201 276 1 3 2 2 1 1 1 1 0 0 0 1 2>&1 | tee ivrserver.out
#mpirun -np 2 ./ivrserver_linux ~damora/Data/CTSkull/skull.vol  256 256 256 1 2 3 2 1 1 2 1 0 0 0 1 2>&1 | tee ivrserver.out
#mpirun -np 1 ./ivrserver_linux ~damora/Data/SEISMIC_B/seismic.B.vol  200 200 100 1 3 2 1 1 1 1 1 1 0 0 0 2>&1 | tee  ivrserver.out

#run for standalone Mac client
#./iVR ~bruced/Data/Shell/vel_grid_small.bin 501 201 276 1 0 2 1 1 1 1 1 0 0 0 1 2>&1 | tee iVR.out
#./iVR  ~bruced/Data/SEISMIC_B/seismic.B.vol  200 200 100 1 0 0 1 1 1 1 1 0 0 0 1 2>&1 | tee  $2.out
