#ifndef TTP_HELPERS_HPP
#define TTP_HELPERS_HPP

#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>

/* HOME = boolean false */
#define HOME false
/* AWAY = boolean true */
#define AWAY true

/* location of a team, either `HOME` or `AWAY` */
typedef bool location;

/*
 * Struct to represent a matchup between two teams.
 *
 * @param team1: the first team in the matchup
 * @param team2: the second team in the matchup
 *
 * @overload operator==: to compare two matchups
 * by checking if team1 and team2 are the same value
 */
struct matchup {
    int team1;
    int team2;

    bool operator==(const matchup& m) const
    {
        return (team1 == m.team1 && team2 == m.team2);
    }
};

/* Hash function for the matchup struct.
 * This is used to store matchups in an unordered_map.
 *
 * @param m: the matchup to hash
 *
 * @return std::size_t: the hash value of the matchup
 */
namespace std {
    template <>
    struct hash<matchup> {
        std::size_t operator()(const matchup &m) const {
            return std::hash<int>()(m.team1) ^ (std::hash<int>()(m.team2) << 1);
        }
    };
}

/*
 * Struct to represent the current streak of playing `HOME` or `AWAY`
 *
 * @param streak_length: the current length of the streak
 * @param loc: the location of the last game in the streak
 */
struct current_streak {
    int streak_length;
    location loc;
};


/*
 * Struct to represent the current streaks of a team.
 *
 * @param homegames: the number of home games played
 * @param awaygames: the number of away games played
 * @param cur_streak: the current streak of the team
 */
struct streak {
    int homegames;
    int awaygames;
    current_streak cur_streak;
};

/*
 * Struct to represent an item in the search tree.
 * Contains the current matchups, schedule, and streaks.
 *
 * @param matchups: the remaining matchups to be scheduled
 * @param schedule: the current schedule
 * @param streaks: the current streaks of each team
 */
struct work_item {
    std::vector<matchup> matchups;
    std::vector<matchup> schedule;
    std::vector<streak> streaks;
};

/*
 * Check if a matchup has already been scheduled in the current round.
 *
 * @param m: the matchup to check
 * @param current: the current matchups in the round
 *
 * @return bool: true if the matchup is a repeat, false otherwise
 */
bool check_repeat(matchup m, std::vector<matchup> current);

/*
 * Check if a team plays the same team back-to-back, which is a noRepeat violation.
 *
 * @param m: the matchup to check
 * @param prev_round: the previous round of matchups
 *
 * @return bool: true if the same teams play eachother back-to-back, false otherwise
 */
bool check_noRepeat(matchup m, std::vector<matchup> prev_round);

/*
 * Check if a team will play the same team back-to-back in the last two rounds.
 *
 * @param macthups: the matchups still to be scheduled
 *
 * @return bool: true if the two teams will play eachother back-to-back, false otherwise
 */
bool check_future_noRepeat(std::vector<matchup> matchups);

/*
 * Check if a team will play more than three games in a row at home or away.
 *
 * @param m: the matchup to check
 * @param streaks: the current streaks of each team
 *
 * @return bool: true if the current team already played at home or the second team already played away three times in a row.
 */
bool check_maxStreak(matchup m, std::vector<streak> streaks);

/*
 * Check if a future streak violation will occur for each team in the current matchup.
 *
 * @param m: the matchup to check
 * @param streaks: the current streaks of each team
 *
 * @return bool: true if the matchup violates the streak constraint, false otherwise
 */
bool future_maxStreak_violation(matchup m, std::vector<streak> streaks);

/*
 * Generate all possible matchups for a given number of teams.
 *
 * @param matchups: the vector to store the generated matchups
 * @param NUM_TEAMS: the number of teams in the schedule
 */
void generate_matchups(std::vector<matchup> &matchups, int NUM_TEAMS);

/*
 * Generate the initial streak count for each team.
 *
 * @param streaks: the vector to store the streak count
 * @param NUM_TEAMS: the number of teams in the schedule
 */
void generate_streak_count(std::vector<streak> &streaks, int NUM_TEAMS);

/*
 * Update the current streaks for the two teams in the current matchup.
 *
 * @param streaks: the current streaks of each team
 * @param m: the current matchup
 */
void update_streaks(std::vector<streak> &streaks, matchup m);

/*
 * Handle the completion of a schedule (does not do anything currently).
 *
 * @param matchups: the current matchups
 * @param NUM_TEAMS: the number of teams in the schedule
 * @param count: the current count of schedules generated
 */
void handle_complete_schedule(std::vector<matchup> &matchups, int NUM_TEAMS, long long int count);

/*
 * Check if a matchup violates any of the following constraints:
 *
 * 1. The matchup is a repeat
 *
 * 2. The same teams play eachother back-to-back
 *
 * 3. A team plays more than three games in a row at home or away
 *
 * 4. A future streak violation will occur for each team in the current matchupq
 *
 * @param schedule: the current schedule
 * @param streaks: the current streaks of each team
 * @param NUM_TEAMS: the number of teams in the schedule
 * @param m: the matchup to check
 *
 * @return bool: true if the matchup violates any constraints, false otherwise
 */
bool check_constraints(std::vector<matchup> &schedule, std::vector<matchup> matchups, std::vector<streak> streaks, int NUM_TEAMS, matchup m);

/* Same as above, but with optimizations */
bool check_constraints_opt(std::vector<matchup> &schedule, std::vector<matchup> matchups, std::vector<streak> streaks, int NUM_TEAMS, matchup m);

/*
 * Create a new work item based on the current item and a matchup.
 *
 * @param current: the current work item
 * @param NUM_TEAMS: the number of teams in the schedule
 * @param m: the matchup to add to the schedule
 *
 * @return work_item: the new work item with the updated schedule
 */
work_item get_new_item(work_item current, int NUM_TEAMS, matchup m);

#endif