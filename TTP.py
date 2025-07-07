from helper import *
import numpy as np

# Generate all possible matchups of teams
def generate_matchups(n, matchups):
    for i in range(n):
        for j in range(n):
            if i != j:
                matchups.append((i, j))
    # Matchups are shuffled to prevent a certain order of teams from occuring every time
    np.random.shuffle(matchups)


# Function to check if a team plays the same team back-to-back
def prevent_back_to_back(m, prev_round):
    if (m[1], m[0]) in prev_round:
        return True
    return False


# Function to generate the home/away (streak) count for each team
def generate_streak_count(n, streaks):
    matches = n-1

    for i in range(n):
        streaks[i] = (matches, matches, (0, "home"))


# Checks if a future streak violation will occur for each team in the current matchup
# This will happen if (x + s) / 3 <= y + 1 is violated for either team
# where x is max(home_games left, away_games left) and y is min(home_games left, away_games left)
# s is the number of games played at home or on the road in a row
def check__future_streak_violation(m, streaks):
    x = max(streaks[m[0]][0] - 1, streaks[m[0]][1])
    y = min(streaks[m[0]][0] - 1, streaks[m[0]][1])
    s = streaks[m[0]][2][0] if (streaks[m[0]][2][1] == "home" and streaks[m[0]][0] - 1 > streaks[m[0]][1]) else 0

    if (x + s) / 3 > y + 1:
        return True

    x = max(streaks[m[1]][0], streaks[m[1]][1] - 1)
    y = min(streaks[m[1]][0], streaks[m[1]][1] - 1)
    s = streaks[m[1]][2][0] if (streaks[m[1]][2][1] == "away" and streaks[m[1]][1] - 1 > streaks[m[1]][0]) else 0

    if (x + s) / 3 > y + 1:
        return True
    return False


# Function to check if the teams in the current matchups will play at home or on the road more than three times in a row
def prevent_four_in_a_row(m, streaks):
    # Get the current streaks for the two teams in the matchup
    streakA = streaks[m[0]][2]
    streakB = streaks[m[1]][2]

    # If the first team already played at home or, the second team on already played on the road three times in a row, skip this round
    if (streakA[0] == 3 and streakA[1] == "home") or (streakB[0] == 3 and streakB[1] == "away"):
        return True
    return False


# Updates the current home/away streaks for the two teams in the current matchup
def update_streaks(m, streaks):
    # Checks whether the first team in the matchup is playing at home or on the road and updates accordingly
    # Also keeps track of the number of home/away games left for each team
    if streaks[m[0]][2][1] == "home":
        streaks[m[0]] = (streaks[m[0]][0]-1, streaks[m[0]][1], (streaks[m[0]][2][0] + 1, "home"))
    else:
        streaks[m[0]] = (streaks[m[0]][0]-1, streaks[m[0]][1], (1, "home"))

    # Same here but for the second team in the matchup
    if streaks[m[1]][2][1] == "away":
        streaks[m[1]] = (streaks[m[1]][0], streaks[m[1]][1]-1, (streaks[m[1]][2][0] + 1, "away"))
    else:
        streaks[m[1]] = (streaks[m[1]][0], streaks[m[1]][1]-1, (1, "away"))


# Generate all possible normalized schedules given all possible rounds
def generate_normalized_schedules(n, matchups, streaks, schedules, args, schedule=[]):
    first_round = []

    # Order the first round in the schedule to normalize it
    for i in range(0, n, 2):
        first_round.append((i, i+1))
        matchups.remove((i, i+1))
        update_streaks((i, i+1), streaks)

    # Generate all possible schedules given this normalized first round
    generate_schedules(n, matchups, streaks, schedules, args, first_round)


# Checks if a team in the current matchup is already playing in the current round
def check_repeat(m, current):
    # Also puts the matchup with the team with the lowest index first
    # This prevents duplicate rounds due to order of teams in a round being different
    for p in current:
        if m[0] in p or m[1] in p or m[0] < p[0]:
            return True
    return False


# Function to check if a schedule meets the constraints
def check_constraints(schedule, streaks, n, m):
    # Gets the index to find the matchups in the current round
    index = len(schedule) % (n//2)
    current = schedule[-index:] if index > 0 else []
    if len(schedule) >= n//2:
        prev_round = schedule[-index-(n//2):-index] if index != 0 else schedule[-index-(n//2):]
    else:
        prev_round = []

    # Checks if a team is playing in the current round
    if check_repeat(m, current):
        return True

    # If the current team is playing the same team back-to-back, skip this round
    if prevent_back_to_back(m, prev_round) if prev_round else False:
        return True

    # If the last three rounds have a team playing at home or away three times in a row, skip this round
    if prevent_four_in_a_row(m, streaks):
        return True
    
    # If the current matchup creates a future streak violation, skip this round
    if check__future_streak_violation(m, streaks):
        return True

    return False


# Function to handle completed schedules
def handle_complete_schedule(n, schedule, schedules, args):
    # Handle the schedule if counter is provided
    if args.count != None and args.random == None:
        if args.count != 0 and get_count() % args.count == 0:
            print("Current schedule count:", get_count())
            save_count(n, args, prefix="")

    # Handle the schedule if verbose is provided
    if args.verbose != None and len(schedules) < args.verbose:
        schedules.append(schedule)

    # Handle the schedule if save is provided
    if args.save != None:
        handle_save(n, schedule, args)


# Generate all possible schedules given all possible matchups
def generate_schedules(n, matchups, streaks, schedules, args, schedule=[]):
    # If the maximum number of schedules has been reached, return
    if len(schedules) == args.max or get_count() == args.max:
        return

    # If there are no more matchups, the schedule is complete
    if len(matchups) == 0:
        counter()
        handle_complete_schedule(n, schedule, schedules, args)
        return

    # For each round still possible, generate a new schedule
    for m in matchups:
        # If the maximum number of schedules has been reached, return
        if len(schedules) == args.max or get_count() == args.max:
            return
        # Checks if a team plays back to back or a team is on the road or at home more than three times in a row
        if check_constraints(schedule, streaks, n, m):
            continue

        new_matchups = [new_m for new_m in matchups if new_m != m]
        new_schedule = [s for s in schedule] + [m]
        new_streaks = streaks.copy()

        update_streaks(m, new_streaks)

        # Generate all possible schedules given this round
        generate_schedules(n, new_matchups, new_streaks, schedules, args, new_schedule)


# Function to generate valid TTP schedules
def generate_TTP(n, args=None):
    schedules = []
    matchups = []
    streaks = {}

    # Create the folder path if save is provided
    # We don't do this when random is provided because
    # otherwise the folder is created and overwitten each time
    if args.save != None and args.random == None and not args.append:
        init_save(n, args)

    # Generate all possible matchups given n teams
    generate_matchups(n, matchups)

    # Fills the streaks dict with the number of home/away games for each team
    # and the number of back-to-back games counter
    generate_streak_count(n, streaks)

    # Generate all possible schedules given all possible rounds
    if args.normalize:
        generate_normalized_schedules(n, matchups, streaks, schedules, args)
    else:
        generate_schedules(n, matchups, streaks, schedules, args)

    # Print the schedules if verbose is provided
    if args.verbose != None:
        print_schedules(n, schedules)

    # Print the number schedules
    if args.count != None and args.verbose == None:
        print(f"Final schedule count ({n} teams): {get_count()}")
        save_count(n, args, prefix="")
    
    # Reset the counter in case of consecutive runs
    reset_count()


# Function to handle the random argument
# Simply uses generate_TTP to generate one random schedule by setting max to 1
# This is repeated random times, to generate a set of random schedules
def handle_random(n, args):
    # Create the folder path if save is provided
    # We do this now to prevent the folder from being
    # overwritten each time generate_TTP is called
    if args.save != None and not args.append:
        init_save(n, args)

    # Set max to 1 to generate one schedule at a time
    # This is why args.max can't be used for random schedules
    args.max = 1

    # Generate a "args.random" amount of schedules
    for i in range(args.random):
        if args.count != None and args.count != 0 and i % args.count == 0:
            print("Current schedule count:", i)

        generate_TTP(n, args)


# Main function takes care of random sampling or normal TTP generation
def main(n, args=None):
    if args.random != None:
        handle_random(n, args)
    else:
        generate_TTP(n, args)