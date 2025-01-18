#!/bin/bash

# SIMULATIONS PARAMETERS
default_n=0
default_rep=100
default_procs=0
# END OF SIMULATIONS PARAMETERS

gcc --version
is_integer() {
  [[ "$1" =~ ^[0-9]+$ ]]
}

run_simulations() {
  echo ""; echo "Executing programs..."
  echo ""; echo "sequential.o"; ./sequential.o "$n" "$rep"
  echo ""; echo "sequential_block.o"; ./sequential_block.o "$n" "$rep"

  if [[ $procs -eq 0 ]]; then
    for ((p = 1 ; p <= 16 ; p*=2 )); do
      echo ""; echo "mpi.o"; mpirun -np $p ./mpi.o "$n" "$rep"
      echo ""; echo "mpi_divided.o"; mpirun -np $p ./mpi_divided.o "$n" "$rep"
      echo ""; echo "mpi_custom_datatypes.o"; mpirun -np $p ./mpi_custom_datatypes.o "$n" "$rep"
      echo ""; echo "mpi_block.o"; mpirun -np $p ./mpi_block.o "$n" "$rep"
    done
  else
    echo ""; echo "mpi.o"; mpirun -np 1 ./mpi.o "$n" "$rep"
    echo ""; echo "mpi_divided.o"; mpirun -np 1 ./mpi_divided.o "$n" "$rep"
    echo ""; echo "mpi_custom_datatypes.o"; mpirun -np 1 ./mpi_custom_datatypes.o "$n" "$rep"
    echo ""; echo "mpi_block.o"; mpirun -np 1 ./mpi_block.o "$n" "$rep"

    echo ""; echo "mpi.o"; mpirun -np $procs ./mpi.o "$n" "$rep"
    echo ""; echo "mpi_divided.o"; mpirun -np $procs ./mpi_divided.o "$n" "$rep"
    echo ""; echo "mpi_custom_datatypes.o"; mpirun -np $procs ./mpi_custom_datatypes.o "$n" "$rep"
    echo ""; echo "mpi_block.o"; mpirun -np $procs ./mpi_block.o "$n" "$rep"
  fi
}

n=""
rep=""
procs=""
if [[ $# -eq 0 ]]; then
  n=$default_n
  rep=$default_rep
  procs=$default_procs

elif [[ $# -eq 1 ]]; then
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  rep=$default_rep
  procs=$default_procs

elif [[ $# -eq 2 ]]; then
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  
  if is_integer "$2" && (($2 > 0)); then
    rep="$2"
  else
    rep=$default_rep
  fi
  procs=$default_procs

else 
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  
  if is_integer "$2" && (($2 > 0)); then
    rep="$2"
  else
    rep=$default_rep
  fi
  if is_integer "$3" && (($3 > 0)); then
    procs="$3"
  else
    procs=$default_procs
  
  fi
fi

echo "Simulation variables"
echo "n=$n"
echo "rep=$rep"
echo "procs=$procs"

mkdir -p bin

cd lib

echo ""
echo "Compiling..."

gcc sequential.c -o ../bin/sequential.o -lm
gcc sequential_block.c -o ../bin/sequential_block.o -lm
mpicc mpi.c -o ../bin/mpi.o -lm
mpicc mpi_divided.c -o ../bin/mpi_divided.o -lm
mpicc mpi_custom_datatypes.c -o ../bin/mpi_custom_datatypes.o -lm
mpicc mpi_block.c -o ../bin/mpi_block.o -lm

echo "Compiling executed correctly!"

cd ../bin
echo ""; echo "Preparing results files..."

rm -f cpu_specs
touch cpu_specs

lscpu > cpu_specs

rm -f results_mpi.csv
touch results_mpi.csv
echo "code,n,processes,time1message,time1effective,time2message,time2effective" > results_mpi.csv

echo "Done!"

if [[ $n -eq 0 ]]; then 
  echo ""; echo "n is set to 0. Running simulations from n=4 to n=12 and rep=$rep"
  for ((i = 4 ; i < 13 ; i++ )); do
    n=$i
    run_simulations
  done
else
  echo ""; echo "Running simulations for n=$n and rep=$rep..."
  run_simulations
fi

echo "Simulations over!"
echo ""; echo "Saving results in \"/results/\"..."
cd ..
mkdir -p results
cd results
rm -f cpu_specs
rm -f results_mpi.csv
mv ../bin/cpu_specs ./cpu_specs
mv ../bin/results_mpi.csv ./results_mpi.csv
echo "All done!"