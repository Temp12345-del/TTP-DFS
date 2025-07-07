#include "TTP_helpers.hpp"

bool check_repeat(matchup m, std::vector<matchup> current) {
    /* Also put the matchup with the lowest index first.
     * This prevents duplicate rounds due to order of teams in a round being different. */
    for (matchup p : current) {
        if ((m.team1 == p.team1 || m.team1 == p.team2) ||
            (m.team2 == p.team1 || m.team2 == p.team2) ||
            (m.team1 < p.team1)) {
            return true;
        }
    }
    return false;
}

bool check_noRepeat(matchup m, std::vector<matchup> prev_round) {
    for (matchup p : prev_round) {
        if (m.team2 == p.team1 && m.team1 == p.team2) {
            return true;
        }
    }
    return false;
}

bool check_future_noRepeat(std::vector<matchup> matchups) {
    for (matchup m : matchups) {
        for (matchup p : matchups) {
            if (m.team2 == p.team1 && m.team1 == p.team2) {
                return true;
            }
        }
    }
    return false;
}

bool check_maxStreak(matchup m, std::vector<streak> streaks) {
    /* Get the current streaks for the two teams in the matchup */
    current_streak streakA = streaks[m.team1].cur_streak;
    current_streak streakB = streaks[m.team2].cur_streak;

    /* If the first team already played at home or the second team already 
     * played away three times in row, skip this round */
    if ((streakA.streak_length == 3 && streakA.loc == HOME) ||
        (streakB.streak_length == 3 && streakB.loc == AWAY)) {
        return true;
    }

    return false;
}

bool future_maxStreak_violation(matchup m, std::vector<streak> streaks) {
    /* A future streak violation will happen if (x + s) / 3 > y + 1 for either team in the matchup
     * where x is max(home_games left, away_games left) and y is min(home_games left, away_games left)
     * s is the number of games played at home or on the road in a row*/
    int x = std::max(streaks[m.team1].homegames - 1, streaks[m.team1].awaygames);
    int y = std::min(streaks[m.team1].homegames - 1, streaks[m.team1].awaygames);
    int s = (streaks[m.team1].cur_streak.loc == HOME &&
             streaks[m.team1].homegames - 1 >  streaks[m.team1].awaygames) ?
                 streaks[m.team1].cur_streak.streak_length : 0;

    if ((x + s) / 3 > y + 1) {
        return true;
    }

    x = std::max(streaks[m.team2].homegames, streaks[m.team2].awaygames - 1);
    y = std::min(streaks[m.team2].homegames, streaks[m.team2].awaygames - 1);
    s = (streaks[m.team2].cur_streak.loc == AWAY &&
         streaks[m.team2].awaygames - 1 >  streaks[m.team2].homegames) ?
             streaks[m.team2].cur_streak.streak_length : 0;

    if ((x + s) / 3 > y + 1) {
        return true;
    }

    return false;
}

void generate_matchups(std::vector<matchup> &matchups, int NUM_TEAMS) {
    int index = 0;
    for (int team1 = 0; team1 < NUM_TEAMS; team1++) {
        for (int team2 = 0; team2 < NUM_TEAMS; team2++) {
            if (team1 != team2) {
                matchup m = {team1, team2};
                matchups[index++] = m;
            }
        }
    }
}

void generate_streak_count(std::vector<streak> &streaks, int NUM_TEAMS) {
    int matches = NUM_TEAMS - 1;
    for (int i = 0; i < NUM_TEAMS; i++) {
        streaks[i].homegames = matches;
        streaks[i].awaygames = matches;
        streaks[i].cur_streak.streak_length = 0;
        streaks[i].cur_streak.loc = HOME;
    }
}

void update_streaks(std::vector<streak> &streaks, matchup m) {
    /* Check if the first team in the matchup is playing at home or away and updates accordingly */
    if (streaks[m.team1].cur_streak.loc == HOME) {
        streaks[m.team1].cur_streak.streak_length++;
    } else {
        streaks[m.team1].cur_streak.streak_length = 1;
    }

    /* Check if the second team in the matchup is playing at home or away and updates accordingly */
    if (streaks[m.team2].cur_streak.loc == AWAY) {
        streaks[m.team2].cur_streak.streak_length++;
    } else {
        streaks[m.team2].cur_streak.streak_length = 1;
    }


    /* Keep track of the number of hame/away games left for each team */
    streaks[m.team1].homegames--;
    streaks[m.team2].awaygames--;

    /* Update the location of the current streak for each team */
    streaks[m.team1].cur_streak.loc = HOME;
    streaks[m.team2].cur_streak.loc = AWAY;
}

void handle_complete_schedule(std::vector<matchup> &matchups, int NUM_TEAMS, long long int count) {
    // printf("Current schedule count: %lld\n", count);
}

bool check_constraints(std::vector<matchup> &schedule, std::vector<matchup> matchups, std::vector<streak> streaks, int NUM_TEAMS, matchup m) {
    /* Get the index to find the matchups in the current round */
    int round_size = NUM_TEAMS / 2;
    int index = schedule.size() % (round_size);

    /* Extract a subvector of the last {index} elements of the vector */
    std::vector<matchup>::const_iterator first = schedule.end() - index;
    std::vector<matchup>::const_iterator last = schedule.end();
    std::vector<matchup> current(first, last);

    /* Check if a team is playing in the current round */
    if (check_repeat(m, current)) {
        return true;
    }

    /* Check if a team is playing back-to-back */
    if (schedule.size() >= round_size) {
        first = schedule.end() - index - round_size;
        last = schedule.end() - index;
        std::vector<matchup> prev_round(first, last);

        if (check_noRepeat(m, prev_round)) {
            return true;
        }
    }

    /* Check four-in-a-row and future streak violations. */
    /* Possible minor optimization: change order. */
    return check_maxStreak(m, streaks);
}

bool check_constraints_opt(std::vector<matchup> &schedule, std::vector<matchup> matchups, std::vector<streak> streaks, int NUM_TEAMS, matchup m) {
    int round_size = NUM_TEAMS / 2;
    int max_size = round_size * (NUM_TEAMS - 1) * 2;
    
    if (check_constraints(schedule, matchups, streaks, NUM_TEAMS, m)) {
        return true;
    }

    /* Calculates whether a maxStreak violation will occur for either team in matchup m */
    if (future_maxStreak_violation(m, streaks)) {
        return true;
    }

    /* Checks if a noRepeat violation will occur in the last 2 rounds */
    if (schedule.size() == max_size - 2 * round_size) {
        if (check_future_noRepeat(matchups)) {
            return true;
        }
    }

    return false;
}

work_item get_new_item(work_item current, int NUM_TEAMS, matchup m) {
    /* Copy matchups, and remove m from the copy. */
    std::vector<matchup> new_matchups(current.matchups);
    std::vector<matchup>::iterator position = std::find(new_matchups.begin(), new_matchups.end(), m);
    if (position != new_matchups.end()) // new_matchups.end() means the element was not found
        new_matchups.erase(position);

    /* Copy schedule and add m to the copy. */
    std::vector<matchup> new_schedule(current.schedule);
    new_schedule.push_back(m);

    /* Copy streaks. */
    std::vector<streak> new_streaks(current.streaks);

    /* Update the streaks for the two teams in the current matchup. */
    update_streaks(new_streaks, m);

    /* Create the new work item. */
    work_item new_item = {new_matchups, new_schedule, new_streaks};

    return new_item;
}