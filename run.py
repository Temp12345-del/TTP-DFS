from TTP import *
from helper import *
import sys
import argparse
import timeit


# Function to validate the arguments and their values
def validate_arguments(args):
    # Check if n_end is provided, if not set it equal to n_start
    if args.n_end == None:
        args.n_end = args.n_start

    # Check if n_start or n_end is odd, TTP is only possible for even n
    if args.n_start % 2 != 0 or args.n_end % 2 != 0:
        print("Number of teams must be even")
        sys.exit(1)
    # Check if n_start is less than n_end
    elif args.n_start > args.n_end:
        print("n_start must be less than or equal to n_end")
        sys.exit(1)
    # Check if n_start is less than 4, TTP is only possible for n >= 4
    elif args.n_start < 4:
        print("Number of teams must be greater than or equal to 4")
        sys.exit(1)
    # Check if verbose is less than or equal to 0
    elif args.verbose != None and args.verbose <= 0:
        print("Verbose must be greater than 0")
        sys.exit(1)
    # Check if count is less than 0
    elif args.count and args.count < 0:
        print("Count must be greater than or equal to 0")
        sys.exit(1)
    # Check if max is less than 0
    elif args.max and args.max < 0:
        print("Max must be greater than or equal to 0")
        sys.exit(1)
    # Check if random is less than 0
    elif args.random and args.random < 0:
        print("Random must be greater than or equal to 0")
        sys.exit(1)
    # Check whether random and max are both provided
    elif args.random and args.max:
        print("Random and max cannot be used together")
        sys.exit(1)
    # Check whether append is provided without save
    elif args.save == None and args.append:
        print("Save and append cannot be used together")
        sys.exit(1)
    # Not implemented warning for parallel processing
    elif args.parallel:
        print("Parallel processing not yet implemented!")
        sys.exit(1)


# Function to parse the arguments from the command line
def parse_arguments():
    parser = argparse.ArgumentParser(description="Generate all possible TTP schedules for n teams")
    # Required arguments
    parser.add_argument("n_start", type=int, help="Number of teams for schedules should be generated, must be even. If n_end is not provided, schedules are generated for n_start only")
    parser.add_argument("n_end", type=int, nargs="?", help="Upper bound for range of teams for which schedules should be generated, must be even. If not provided, schedules are generated for n_start only")
    # Optional boolean arguments
    parser.add_argument("-N", "--normalize", action="store_true", help="Generate normalized schedules")
    parser.add_argument("-p", "--parallel", action="store_true", help="Enable parallel processing (not implemented)") # Not implemented
    parser.add_argument("--append", action="store_true", help="Append schedules to the file instead of overwriting. Only works with --save")
    # Optional non-boolean arguments
    parser.add_argument("-v", "--verbose", type=int, help="Prints first VERBOSE rounds of all schedules, possible rounds and matchups")
    parser.add_argument("-c", "--count", type=int, help="Print the count of schedules generated. Every COUNT schedules, the count is printed. Set to 0 to only print the final count")
    parser.add_argument("-m", "--max", type=int, help="Maximum number of schedules to generate")
    parser.add_argument("-s", "--save", type=str, help="Saves the schedules to a given file")
    parser.add_argument("-r", "--random", type=int, help="Generate random schedules by restarting the algorithm with a different initial matchup order each time")
    parser.add_argument("-t", "--timer", action="store_true", help="Time the generation of schedules")
    return parser.parse_args()


# Times the execution of the TTP algorithm for a given n
def timer(n, args):
    # Running and timing the generation of all Latin Squares of order n
    start_time = timeit.default_timer()
    main(n, args)
    stop_time = timeit.default_timer()
    runtime = stop_time - start_time

    # Format the time
    if runtime < 1:
        time = f"{runtime * 1000:.2f} ms"
    elif runtime < 60:
        time = f"{runtime:.2f} s"
    elif runtime < 3600:
        time = f"{runtime / 60:.2f} min"
    elif runtime < 3600 * 24:
        time = f"{runtime / 3600:.2f} h"
    else:
        time = f"{runtime / (3600 * 24):.2f} days"

    return time


# Main function to run the program
def run():
    # Get the arguments from the command line
    args = parse_arguments()

    # Validate the arguments
    validate_arguments(args)

    # Run generate_TTP for each n in the range
    for n in range(args.n_start, args.n_end + 1, 2):
        print(f"Generating schedules for {n} teams")

        # Measures and prints the time taken for each n, if timer is enabled
        if args.timer:
            time = timer(n, args)
            print(f"Time taken: {time}\n")
        else:
            main(n, args)


if __name__ == "__main__":
    run()