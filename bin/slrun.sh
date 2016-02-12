#!/bin/bash
export OMP_NUM_THREADS=2

if [ -z "$SLURM_JOBID" ]; then
	sbatch --gid=`hostname -s` --time=15:00  --nodes=128 --ntasks-per-node=16   -O --qos=umax-128 $0
	#sbatch --gid=`hostname -s` --time=5:00 --nodes=32 --ntasks-per-node=16  -O --qos=umax-32 $0
else
	srun --ntasks-per-node=16  --chdir=/gpfs/DDNgpfs1/damora/BGQ/ivr/bin ./ivrserver /gpfs/DDNgpfs1/damora/BGQ/VISFEMALE/vizwoman8.dat  1024 608 2594 0 -1 5 8 16 16 8 8 0 1 0 0
#	srun --ntasks-per-node=16  --chdir=/gpfs/DDNgpfs1/damora/BGQ/ivr/bin ./ivrserver /gpfs/DDNgpfs1/damora/BGQ/Shell/vel_grid_large.bin  2002 802 1102 1 0 2 16 8 8 8 8 0 1 0 0
fi

#srun --ntasks-per-node=16  --chdir=/gpfs/DDNgpfs1/damora/BGQ/ivr/bin ./ivrserver /gpfs/DDNgpfs2/fossum/isoparm/vizwoman8.dat  2002 802 1102 1 0 2 8 8 8 8 8 0 1 0 0

# for reserved partition 
#sbatch --gid=`hostname -s` --time=60:00 --nodes=512 --reservation=bruce0701  -O  --qos=umax-512 $0
#srun --ntasks-per-node=32  --chdir=/gpfs/DDNgpfs2/fossum/isoparm  ./isoparm_server.JUN30


#sbatch --gid=`hostname -s` --time=5:00 --nodes=512 --ntasks-per-node=32  -O --qos=umax-512 $0
#srun  --chdir=/gpfs/DDNgpfs2/fossum/isoparm  ./isoparm_server.JUN30

#sbatch --gid=`hostname -s` --time=5:00 --nodes=32 --ntasks-per-node=16  -O --qos=umax-512 $0
#srun  --chdir=/gpfs/DDNgpfs2/fossum/isoparm/isoparm ./isoparm_server 

#sbatch --gid=`hostname -s` --time=5:00 --nodes=32 --ntasks-per-node=16  -O --qos=umax-32 $0
#srun --ntasks-per-node=16  --chdir=/gpfs/DDNgpfs1/damora/BGQ/ivr/bin ./ivrserver /gpfs/DDNgpfs1/damora/BGQ/Shell/vel_grid_large.bin  2002 802 1102 1 0 2 8 8 8 8 8 0 1 0 0

#sbatch --gid=`hostname -s` --time=5:00 --nodes=32 --ntasks-per-node=16  -O --qos=umax-32 $0
#srun --ntasks-per-node=16  --chdir=/gpfs/DDNgpfs1/damora/BGQ/ivr/bin ./ivrserver /gpfs/DDNgpfs1/damora/BGQ/VISFEMALE/vizwoman8.dat  1024 608 2594 1 0 5 8 8 8 8 8 0 1 0 0
