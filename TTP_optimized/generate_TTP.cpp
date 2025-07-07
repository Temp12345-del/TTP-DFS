#include "plf_stack.h"
#include "generate_TTP.hpp"
#include "parallel_TTP.hpp"
#include "TTP_helpers.hpp"
#include <chrono>
#include <mpi.h>
#include <omp.h>

/* The maximum value for a long long int. */
#define MAX_COUNT 9223372036854775807

/* Global variable to keep track of the number of schedules generated. */
long long int count = 0;


void generate_schedules(work_item item, int num_teams, long long int max) {
    /* Check if the maximum number of schedules has been reached */
    if (count == max) {
        return;
    }

    /* If there are no more matchups, the schedule is complete */
    if (item.matchups.size() == 0) {
        count++;
        return;
    }

    /* For each round still possible, generate a new schedule */
    for (matchup m : item.matchups) {
        /* Check if the maximum number of schedules has been reached */
        if (count == max) {
            return;
        }

        /* Check validity of the schedule with constraints */
        if (check_constraints(item.schedule, item.matchups, item.streaks, num_teams, m)) {
            continue;
        }

        /* Create a new work item based on the current item and a matchup */
        work_item new_item = get_new_item(item, num_teams, m);

        /* Recursive call: generate all possible schedules given the new round */
        generate_schedules(item, num_teams, max);
    }
}

void generate_schedules_stack(work_item item, int num_teams, long long int max) {
    /* Initialize the stack and push the first item */
    plf::stack<work_item> stack = plf::stack<work_item>();
    stack.push(item);

    /* Continue until the stack is empty */
    while (!stack.empty()) {
        /* Get the top item from the stack */
        work_item current = stack.top();
        stack.pop();

        /* Check if the maximum number of schedules has been reached */
        if (count == max) {
            return;
        }

        /* If there are no more matchups, the schedule is complete */
        if (current.matchups.size() == 0) {
            count++;
            continue;
        }

        /* For each round still possible, generate a new schedule */
        for (matchup m : current.matchups) {
            /* Check if the maximum number of schedules has been reached */
            if (count == max) {
                return;
            }

            /* Check validity of the schedule with constraints */
            if (check_constraints(current.schedule, current.matchups, current.streaks, num_teams, m)) {
                continue;
            }

            /* Create a new work item based on the current item and a matchup and push it to the stack */
            work_item new_item = get_new_item(current, num_teams, m);
            stack.push(new_item);
        }
    }
}

void generate_normalized_schedules(work_item item, int num_teams, long long int max, int bfs_iter) {
    std::vector<matchup> first_round = std::vector<matchup>();

    /* Order the first round in the schedule to normalize it */
    for (int i = 0; i < num_teams; i += 2) {
        first_round.push_back({i, i + 1});
        matchup m = {i, i + 1};
        std::vector<matchup>::iterator position = std::find(item.matchups.begin(), item.matchups.end(), m);
        item.matchups.erase(position);
        update_streaks(item.streaks, {i, i + 1});
    }

    count = generate_schedules_mpi_dynamic({item.matchups, first_round, item.streaks}, num_teams, max, bfs_iter);
}

int main(int argc, char* argv[]) {
    /* Initialize MPI */
    MPI_Init(NULL, NULL);

    int world_rank, world_size, namelen;
    char node_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Get_processor_name(node_name, &namelen);

    /* Check if the correct number of arguments is given */
    if (argc < 3) {
        printf("Usage: ./run <num_teams> <bfs_iter> [MAX]\n");
        return 1;
    }

    /* Parse the command line arguments */
    int num_teams = atoi(argv[1]);
    int bfs_iter = atoi(argv[2]);
    long long int max = argc > 5 ? atoi(argv[5]) : MAX_COUNT;

    auto start_time = std::chrono::high_resolution_clock::now();

    /* Initialize the matchups, schedule, and streaks */
    auto matchups = std::vector<matchup>(num_teams * (num_teams - 1));
    auto schedule = std::vector<matchup>();
    auto streaks = std::vector<streak>(num_teams);

    /* Generate the initial matchups */
    generate_matchups(matchups, num_teams);

    /* Generate the initial streak count */
    generate_streak_count(streaks, num_teams);

    /* Count the normalized schedules */
    generate_normalized_schedules({matchups, schedule, streaks}, num_teams, max, bfs_iter);

    /* Calculate the elapsed time */
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    if (world_rank == 0) {
        /* Print the final schedule count */
        printf("Final schedule count (%d teams): %lld\n", num_teams, count);
        /* Print the time in HH:MM:SS:MS format */
        printf("Time taken: %d:%d:%d:%d\n", (int)elapsed.count() / 3600, (int)elapsed.count() % 3600 / 60, (int)elapsed.count() % 60, (int)(elapsed.count() * 1000) % 1000);
    }

    /* Finalize MPI */
    MPI_Finalize();
}