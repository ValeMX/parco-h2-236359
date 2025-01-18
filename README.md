# Introduction to Parallel Computing. Exploring Implicit and Explicit Parallelism with OpenMP
This project aims to deeply explore the use of Parallel Programming techniques in the forms of Implicit-Level Parallelism (ILP) and Explicit Parallelism with OpenMP (OMP) in order to optimize two different problems, the symmetry check and the transpose computation of a matrix, and subsequently benchmark and analyze the performance of both approaches, comparing their efficiency and scalability.

## Description of the repository
```
.
├── bin            # Compiled program files
├── lib            # Source code files
├── results        # Results of the simulations
├──
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
This will start the simulations on the `short_cpuQ` with 1 selected node, 32 cpus and 512 MB of memory. The default simulation parameters are `n=0`, `rep=500`, `threads=0` that can be configured by modifying them in the first lines of the `start.pbs` file. When `n` is set to `0`, all the simulations from `n=4` to `n=12` will be executed; when `rep` is set to an invalid or negative number, it will be assigned to default  `500`; when `threads` is set to `0`, all the simulations from `threads=2` to `threads=64` (doubled at every iteration) will be executed, otherwise only the sequential and the passed `threads` will be executed. At the end of the simulations, the results will be saved in the `results` folder as 2 .csv files: `results_ilp.csv` and `results_omp.csv`.

Contents of `results_ilp.csv`:

| Column      | Description |
| ----------- | ----------- |
| code        | The code assigned to the program (refer to the table above) |
| n | The dimension of the input matrix |
| flops | The computed flops of the symmetry check routine |
| bandwidth | The computed bandwidth in B/s of the transpose routine |

Contents of `results_omp.csv`:

| Column      | Description |
| ----------- | ----------- |
| code        | The code assigned to the program (refer to the table above) |
| n | The dimension of the input matrix |
| threads | The number of threads used |
| speedup1 | The computed speedup of the symmetry check routine |
| efficiency1 | The computed efficiency of the symmetry check routine |
| speedup2 | The computed speedup of the transpose routine |
| efficiency2 | The computed efficiency of the transpose routine |
| bandwidth | The computed bandwidth in B/s of the transpose routine |

In order to execute the simulations on the local system, ensure that `gcc-9.1.0` is installed and execute the script `start.sh [n] [rep] [threads]` from the home folder of the repository. If no or lower than 3 arguments are passed, the missing ones will be assigned as described before with `start.pbs`. At the end of the simulations, the results will be saved in the `results` folder as 2 .csv files: `results_ilp.csv` and `results_omp.csv`.

In order to execute single programs, ensure that `gcc-9.1.0` is installed and comile using the following commands based on the desired source file:

S: sequential.c
```
gcc sequential.c -o sequential.o -lm
```

V: vectorization.c:
```
gcc vectorization.c -o vectorization.o -mavx2 -lm
```

B: block_access_pattern.c:
```
gcc block_access_pattern.c -o ../bin/block_access_pattern.o -lm
```

BP: block_access_pattern_prefetching.c:
```
gcc block_access_pattern_prefetching.c -o ../bin/block_access_pattern_prefetching.o -lm
```

BO1: block_access_pattern.c
```
gcc block_access_pattern.c -o block_access_pattern_O1.o -O1 -lm -DO1
```

BO2: block_access_pattern.c
```
gcc block_access_pattern.c -o block_access_pattern_O2.o -O2 -lm -DO2
```

BO3: block_access_pattern.c
```
gcc block_access_pattern.c -o block_access_pattern_O3.o -O3 -lm -DO3
```

BOf: block_access_pattern.c
```
gcc block_access_pattern.c -o block_access_pattern_Ofast.o -Ofast -lm -DOfast
```

O: omp.c
```
gcc omp.c -o omp.o -fopenmp -lm
```

OR: omp_reduction.c
```
gcc omp_reduction.c -o omp_reduction.o -fopenmp -lm
```

OB: omp_block_access_pattern.c
```
gcc omp_block_access_pattern.c -o omp_block_access_pattern.o -fopenmp -lm
```

OBT: omp_triangular_numbers.c
```
gcc omp_triangular_numbers.c -o omp_triangular_numbers.o -fopenmp -lm
```

OB_S: omp_static_scheduling.c
```
gcc omp_static_scheduling.c -o omp_static_scheduling.o -fopenmp -lm
```

OB_D: omp_dynamic_scheduling.c
```
gcc omp_dynamic_scheduling.c -o omp_dynamic_scheduling.o -fopenmp -lm
```

OBf: omp_dynamic_scheduling.c
```
gcc omp_dynamic_scheduling.c -o omp_dynamic_scheduling_Ofast.o -fopenmp -Ofast -lm -DOfast
```

After executing the compiled `.o` file (with the parameters `n`, `rep` and `threads` for omp codes as explained above), a `.csv` file will be generated in the same folder with the execution results.

## Analyzing results
In order to visualize the produced results, there are three python scripts in the home folder:
- `ilp_table.py`: creates two tables for GFLOPS and Bandwidth in GB/s from data in `results/results_ilp.csv`
- `omp_plotting.py`: plots the four graphs for speedup and efficiency for symmetry check and transpose (for a specific power of two [4..12]) from data in `results/results_omp.csv`
- `bandwidth.py`: plots the graph for effective and theoretical peak bandwidth (for a specific power of two [4..12]) from data in `results/results_omp.csv`

In order to be able to execute the python scripts, it is required to have a version of `python 3` along with the modules `numpy`, `pandas`, `matplotlib` and `tabulate` installed. To execute the scripts, assumin that `python` command is available, execute the following commands from the home folder of the repository:

ilp_table.py
```
python ilp_table.py
```

omp_plotting.py
```
python omp_plotting.py
```

bandwidth.py
```
python bandwidth.py
```
