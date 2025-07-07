#ifndef GENERATE_TTP_HPP
#define GENERATE_TTP_HPP

#include <vector>
#include "TTP_helpers.hpp"

/*
 * Generate all possible schedules recursively, given all possible matchups.
 * Updates the global variable `count` with the number of schedules generated.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 */
void generate_schedules(work_item item, int NUM_TEAMS, long long int max);

/*
 * Generate all possible schedules using a stack, given all possible matchups.
 * Updates the global variable `count` with the number of schedules generated.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 */
void generate_schedules_stack(work_item item, int NUM_TEAMS, long long int max);
/*
 * Generate all possible normalized schedules given all possible rounds.
 * Updates the global variable `count` with the number of schedules generated.
 * Also calls the correct parallel function based on the number of nodes and threads.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 * @param num_nodes: number of compute nodes to use for MPI (if 1, do not use MPI)
 * @param num_threads: number of parallel threads to use per node for openMP (if 0, run recursive function, if 1 run stack, if > 1 run parallel)
 * @param thread_iterations: number of iterations to use for the BFS section of the parallel function
 */
void generate_normalized_schedules(work_item item, int NUM_TEAMS, long long int max, int num_nodes, int num_threads, int thread_iterations);
#endif