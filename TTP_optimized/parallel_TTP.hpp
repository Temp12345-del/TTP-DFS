#ifndef PARALLEL_TTP_HPP
#define PARALLEL_TTP_HPP
#include <vector>
#include <unordered_map>
#include "generate_TTP.hpp"
#include "TTP_helpers.hpp"

/*
 * Execute the `generate_schedules` function in parallel,
 * parallelizing over multiple nodes using MPI,
 * with each node running the `generate_schedules_openmp` function.
 *
 * @param item: work_item struct containing the matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 * @param num_nodes: number of compute nodes to use for MPI
 * @param threads_per_node: number of parallel threads to use per node for openMP
 *
 * @return long long int: the number of schedules generated
 */
long long int generate_schedules_mpi(work_item item, int num_teams, long long int max, int bfs_iter);

long long int generate_schedules_mpi_dynamic(work_item item, int num_teams, long long int max, int bfs_iter);

#endif