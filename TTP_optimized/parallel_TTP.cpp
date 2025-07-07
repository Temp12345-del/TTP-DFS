#include "plf_stack.h"
#include "parallel_TTP.hpp"
#include "TTP_helpers.hpp"
#include <omp.h>
#include <chrono>
#include <mpi.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <numeric>

long long int local_count = 0;
// #pragma omp threadprivate(count_test)

/*
 * Create a list of tasks (work_items) to be executed in parallel,
 * using BFS to explore the first few layers of the search tree.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param iterations: number of iterations to run the BFS (layers of the search tree)
 * @param count: in case a valid schedule is found, increase the count
 * @param num_teams: number of teams in the schedule
 *
 * @return std::vector<work_item>: list of work_items for parallel execution
 */
std::vector<work_item> create_task_list(work_item item, int iterations, long long int &count, int num_teams) {
    /* Initialize lists with tasks */
    std::vector<work_item> tasks = std::vector<work_item>();
    std::vector<work_item> new_tasks = std::vector<work_item>();
    tasks.push_back(item);

    /* Create the list of tasks by BFS. */
    for (int i = 0; i < iterations; i++) {
        for (work_item item : tasks) {
            /* If a valid schedule is found, increase the count and continue */
            if (item.matchups.size() == 0) {
                count++;
                continue;
            }

            /* For each round still possible, generate a new schedule */
            for (matchup m : item.matchups) {
                /* Check validity of the schedule with constraints. */
                if (check_constraints(item.schedule, item.matchups, item.streaks, num_teams, m)) {
                    continue;
                }

                /* Push the work item to the list. */
                work_item new_item = get_new_item(item, num_teams, m);
                new_tasks.push_back(new_item);
            }
        }

        /* Task becomes new tasks, and new tasks is emptied for the next iteration. */
        tasks = new_tasks;
        new_tasks.clear();
    }

    return tasks;
}

/*
 * Continue the search of valid schedules in a subtree starting from a specific node.
 * This function is used for the parallel execution of the search, and uses recursion.
 *
 * @param item: work_item struct containing the specific node to start the search
 * @param num_teams: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 *
 * @return long long int: the number of valid schedules in the searched subtree
 */
void thread_func_rec(work_item item, int num_teams, long long int max) {
    /* Check if the maximum number of schedules has been reached */
    if (local_count == max) {
        return;
    }

    /* If there are no more matchups, the schedule is complete */
    if (item.matchups.size() == 0) {
        local_count++;
        return;
    }

    /* For each round still possible, generate a new schedule */
    for (matchup m : item.matchups) {
        /* Check if the maximum number of schedules has been reached */
        if (local_count == max) {
            return;
        }

        /* Check validity of the schedule with constraints */
        if (check_constraints(item.schedule, item.matchups, item.streaks, num_teams, m)) {
            continue;
        }

        /* Create a new work item based on the current item and a matchup */
        work_item new_item = get_new_item(item, num_teams, m);

        /* Recursive call: generate all possible schedules given the new round, and add them to local_count */
        thread_func_rec(new_item, num_teams, max);
    }
}

/*
 * Continue the search of valid schedules in a subtree starting from a specific node.
 * This function is used for the parallel execution of the search, and uses recursion.
 * This is an optimized version of the previous function.
 *
 * @param item: work_item struct containing the specific node to start the search
 * @param num_teams: number of teams in the schedule
 * @param max: maximum number of schedules to generate
 *
 * @return long long int: the number of valid schedules in the searched subtree
 */
void thread_func_rec_opt(work_item item, int num_teams, long long int max) {
    /* Check if the maximum number of schedules has been reached */
    if (local_count == max) {
        return;
    }

    /* Checks if the schedule reached the max length */
    if (item.schedule.size() == ((num_teams * (num_teams - 1)))) {
        local_count++;
        return;
    }

    /* For each round still possible, generate a new schedule */
    for (matchup m : item.matchups) {
        /* Check if the maximum number of schedules has been reached */
        if (local_count == max) {
            return;
        }

        /* Check validity of the schedule with constraints */
        if (check_constraints_opt(item.schedule, item.matchups, item.streaks, num_teams, m)) {
            continue;
        }

        /* Mark the matchup as processed and push to schedule */
        item.schedule.push_back(m);
        item.matchups.erase(std::remove(item.matchups.begin(), item.matchups.end(), m), item.matchups.end());

        /* Update the streaks for the two teams in the current matchup */
        streak old_streak1 = item.streaks[m.team1];
        streak old_streak2 = item.streaks[m.team2];
        update_streaks(item.streaks, m);

        /* Recursive call */
        thread_func_rec_opt(item, num_teams, max);

        /* Undo streak mod */
        item.streaks[m.team1] = old_streak1;
        item.streaks[m.team2] = old_streak2;

        /* Undo matchup placement */
        item.schedule.pop_back();
        item.matchups.insert(item.matchups.begin(), m);
    }
}


/*********************************************************
 * MPI Implementation(s). Using multiple nodes,
 * where each node uses multiple threads to maximize utilization.
 * *******************************************************/

/* Option 1: static scheduling. Easiest to implement,
 * but probably not optimal load balancing. */

long long int generate_schedules_mpi(work_item item, int num_teams, long long int max, int bfs_iter) {
    /* Get the ID of the node, and the total number of nodes in the system */
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /* Initialize local count for each node, and total count to be used for reduction */
    long long int total_count = 0;
    std::vector<work_item> tasks;

    /* Create the list of tasks for parallel execution */
    tasks = create_task_list(item, bfs_iter, local_count, num_teams);

    if (world_rank == 0) {
        printf("%d tasks to execute.\n", tasks.size());
    }

    /* Divide the tasks among the nodes */
    int chunk_size = tasks.size() / world_size;
    int remainder = tasks.size() % world_size;
    int start = world_rank * chunk_size + std::min(world_rank, remainder);
    int end = (world_rank + 1) * chunk_size + std::min(world_rank + 1, remainder);
    tasks = std::vector<work_item>(tasks.begin() + start, tasks.begin() + end);

    // Un-optimized
    // for (int i = 0; i < tasks.size(); i++) {
    //     thread_func_rec(tasks[i], num_teams, max);
    // }

    // New optimized
    for (int i = 0; i < tasks.size(); i++) {
        thread_func_rec_opt(tasks[i], num_teams, max);
    }

    printf("Node %d: %lld schedules found.\n", world_rank, local_count);

    /* Reduce the local counts of each node to the total count on the root node */
    MPI_Reduce(&local_count, &total_count, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    /* Finalize MPI environment and return the total count of valid schedules */
    return total_count;
}

/* Option 2: Dynamic scheduling.
 * 1 master thread, handing tasks to all other worker threads. */

long long int generate_schedules_mpi_dynamic(work_item item, int num_teams, long long int max, int bfs_iter) {
    /* Get the ID of the node, and the total number of nodes in the system */
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /* Initialize local count for each node, and total count to be used for reduction */
    long long int total_count = 0;
    std::vector<work_item> tasks;

    /* Create the list of tasks for parallel execution. Small overhead, so do on every node */
    tasks = create_task_list(item, bfs_iter, local_count, num_teams);

    if (world_rank == 0) {
        printf("%d tasks to execute.\n", tasks.size());
    }

    /* Master loop. Keep looping, and every iteration block until another rank wants to get a task.
     * Then send the id of the current_task to that node to process and continue the loop. */
    for (int current_task = 0; world_rank == 0 && current_task < tasks.size() + world_size - 1; current_task++) {
        /* Send the current task to a worker node */
        int worker_rank;
        MPI_Status status;
        MPI_Recv(&worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        /* If the current task is out of range, send -1 to the worker node to indicate no more tasks */
        if (current_task >= tasks.size()) {
            int no_more_tasks = -1;
            MPI_Send(&no_more_tasks, 1, MPI_INT, worker_rank, 0, MPI_COMM_WORLD);
            continue;
        }
        /* Send the task id to the worker node */
        MPI_Send(&current_task, 1, MPI_INT, worker_rank, 0, MPI_COMM_WORLD);
    }

    /* Worker loop. Ask task from master and process it. */
    while (world_rank != 0) {
        /* Request a task from the master node */
        int master_rank = 0;
        MPI_Send(&world_rank, 1, MPI_INT, master_rank, 0, MPI_COMM_WORLD);

        /* Receive the task from the master node */
        int task_id;
        MPI_Status status;
        MPI_Recv(&task_id, 1, MPI_INT, master_rank, 0, MPI_COMM_WORLD, &status);

        /* If the task id is -1, break the loop (no more tasks) */
        if (task_id == -1) {
            break;
        }

        /* Get the task from the list of tasks */
        work_item task = tasks[task_id];

        printf("Node %d: processing task %d.\n", world_rank, task_id);

        /* Process the task */
        thread_func_rec_opt(task, num_teams, max);
    }

    MPI_Reduce(&local_count, &total_count, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    return total_count;
}