generate_TTP.cpp contains most code from the TTP.py file.
Some helper functions, including initialization and validitiy checks, and data structures are implemented in the TTP_helpers.cpp file
Parallel implementations using OpenMP and MPI are in parallel_TTP.cpp.

Benchmark data, and a jupyter notebook with plots are in the benchmarks subdirectory.

To compile, use `make`.
To run, use `./run`.

`./run` takes 4 mandatory, and 1 optional arguments:
- NUM_TEAMS: Number of teams
- NUM_NODES: Number of nodes
- NUM_THREADS: Number of threads
- THREAD_ITER: Number of BFS iterations for task list creation
- MAX: Optional argument to specify the maximum number of iterations for the main loop. Default is maximum 64 bit integer value.

If running on DAS, make sure to use `module load openmpi/gcc/64` to enable MPI compilation.
Furthermore, make sure to use slurm or prun to run experiments. Some sbatch scripts are provided.