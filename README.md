# Introduction to Parallel Computing. Parallelizing matrix operations using MPI
This project aims to deeply explore the use of Parallel Programming techniques in the form of Message Passing Interface (MPI) on a distributed memory system in order to optimize two different problems, the symmetry check and the transpose computation of a matrix, and subsequently benchmark and analyze the performance of the implementations, comparing their efficiency and scalability with respect to the sequential baseline.


## Description of the repository
```
.
├── bin                          # Compiled program files
├── lib                          # Source code files
├── results                      # Results of the simulations
├── 236359_Cassone_Valerio.pdf   # Report file
├── bandwidth.py                 # Python script to plot bandwidth graphs
├── flops.py                     # Python script to plot flops graphs
├── mpi_plotting.py              # Python script to plot speedup and efficiency graphs
├── mpi_scalability.py           # Python script to plot scalability graphs
├── start.pbs                    # Script to run simulations on HPC
├── start.sh                     # Script to run simulations on local machine
└── README.md
```
The repository contains all the files used to run the simulations with the methodologies described in the paper. The `lib` folder contains all the source code files. When compiled via the scripts the generated programs will be located in the `bin` and, once executed, the results will be saved in the `results` folder.
Each source file has an identifying assigned code:

| Code      | File |
| ----------- | ----------- |
| S | `sequential.c` |
| SB | `sequential_block.c` |
| M | `mpi.c` |
| MD | `mpi_divided.c` |
| MC | `mpi_custom_datatypes.c` |
| MB | `mpi_block.c` |


## Instructions for reproducibility
In order to execute on the HPC all the simulations, from the home folder of the repository execute the command:
```
qsub start.pbs
```
This will start the simulations on the `short_cpuQ` with 1 selected node, 16 cpus, 16 mpi processes and 1024 MB of memory. The default simulation parameters are `n=0`, `rep=100`, `procs=0` that can be configured by modifying them in the first lines of the `start.pbs` file. When `n` is set to `0`, all the simulations from `n=4` to `n=12` will be executed; when `rep` is set to an invalid or negative number, it will be assigned to default  `100`; when `procs` is set to `0`, all the simulations from `procs=2` to `procs=16` (doubled at every iteration) will be executed, otherwise only the sequential and the passed `procs` will be executed. At the end of the simulations, the results will be saved in the `results` folder as `results_mpi.csv`.

Contents of `results_mpi.csv`:

| Column      | Description |
| ----------- | ----------- |
| code        | The code assigned to the program (refer to the table above) |
| n | The dimension of the input matrix |
| processes | The number of processes executed |
| time1message | The execution time of the symmetry check routine |
| time1effective | The execution time of the symmetry check routine (excluded the message passing time) |
| time2message | The execution time of the transpose routine |
| time2effective | The execution time of the transpose routine (excluded the message passing time) |

In order to execute the simulations on the local system, ensure that `gcc-9.1.0` is installed along with `mpich-3.2.1` and execute the script `start.sh [n] [rep] [procs]` from the home folder of the repository. If no or lower than 3 arguments are passed, the missing ones will be assigned as described before with `start.pbs`. At the end of the simulations, the results will be saved in the `results` folder as `results_mpi.csv`.

In order to execute single programs, ensure that `gcc-9.1.0` and `mpich-3.2.1` are installed, move in the `lib` folder, compile and execute using the following commands based on the desired source file:

S: sequential.c
```
gcc sequential.c -o sequential.o -lm
./sequential.o [n] [rep]
```

SB: sequential_block.c:
```
gcc sequential_block.c -o sequential_block.o -lm
./sequential_block.o [n] [rep]
```

M: mpi.c:
```
mpicc mpi.c -o mpi.o -lm
mpirun -np [procs] ./mpi.o [n] [rep]
```

MD: mpi_divided.c:
```
mpicc mpi_divided.c -o mpi_divided.o -lm
mpirun -np [procs] ./mpi_divided.o [n] [rep]
```

MC: mpi_custom_datatypes.c:
```
mpicc mpi_custom_datatypes.c -o mpi_custom_datatypes.o -lm
mpirun -np [procs] ./mpi_custom_datatypes.o [n] [rep]
```

MB: mpi_block.c:
```
mpicc mpi_block.c -o mpi_block.o -lm
mpirun -np [procs] ./mpi_block.o [n] [rep]
```

After executing the compiled `.o` file, a `.csv` file will be generated in the same folder with the execution results.

## Analyzing results
In order to visualize the produced results, there are four python scripts in the home folder:
- `bandwidth.py`: plots the graphs for bandwidth for symmetry check and transpose (for a specific power of two [4..12]) from data in `results/results_mpi.csv`
- `flops.py`: plots the graphs for flops for symmetry check and transpose (for a specific power of two [4..12]) from data in `results/results_mpi.csv`
- `mpi_plotting.py`: plots the graphs for speedup and efficiency for symmetry check and transpose (for a specific power of two [4..12]) from data in `results/results_mpi.csv`
- `mpi_scalability.py`: plots the graph for scalability (starting from a specific power of two [4..8]) from data in `results/results_mpi.csv`

In order to be able to execute the python scripts, it is required to have a version of `python 3` along with the modules `numpy`, `pandas`, `matplotlib` and installed. To execute the scripts, assuming that `python` command is available, execute the following commands from the home folder of the repository:

bandwidth.py
```
python bandwidth.py
```

flops.py
```
python flops.py
```

mpi_plotting.py
```
python mpi_plotting.py
```

mpi_scalability.py
```
python mpi_scalability.py
```
