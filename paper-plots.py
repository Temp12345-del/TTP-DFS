import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm, betabinom
from calc import fit_beta_binom

# Constants for font sizes
FONTSIZE = 40
FONTSIZE_SMALL = 28
FONTSIZE_AXIS = 25
FONTSIZE_MAX_DIFF = 22
FONTSIZE_MEAN_DIFF = 26

# Constants for plot types
DIFFERENCES = 0
DIFFERENCES_WITHOUT_HA = 1
DIFFERENCES_ONLY_HA = 2


# Annotate the plot with the max and mean differences,the beta binom stats, and a small title box
def annotate_plot(subplot, n, plot_type, max_y_axis, max_diff, mean_diff, alpha, beta):
    # Add vertical lines for the max difference and annotate
    if n == 4:
        x_coord = 0.95
        y_coord = 0.85
    elif n == 6:
        x_coord = 0.99
        y_coord = 0.8
    elif n == 8:
        x_coord = 0.995
        y_coord = 0.8
    else:
        x_coord = 0.995
        y_coord = 0.85
    subplot.axvline(x=max_diff, color='red', linestyle='--', linewidth=2, label="Max. possible difference")
    subplot.annotate("Maximum\ndifference",
                    xy=(max_diff, 0),
                    xytext=(x_coord, y_coord),
                    xycoords='axes fraction',
                    ha='right',
                    fontstyle='italic',
                    fontsize=FONTSIZE_MAX_DIFF)

    # Add vertical lines for the mean difference and annotate it
    if plot_type == DIFFERENCES_ONLY_HA:
        y_coord = max_y_axis * 0.2
    elif n == 4:
        y_coord = max_y_axis * 0.2
    elif n == 6:
        y_coord = max_y_axis * 0.1
    elif n == 8:
        y_coord = max_y_axis * 0.15
    else:
        y_coord = max_y_axis * 0.1
    subplot.axvline(x=mean_diff, color='green', linestyle='--', linewidth=2)
    subplot.annotate(rf"$\mu$: {mean_diff:.2f}",
                    xy=(mean_diff, 0),
                    xytext=(mean_diff, y_coord),
                    ha='center',
                    zorder=100,
                    bbox=dict(facecolor='#4DFE90', edgecolor='black', boxstyle='round,pad=0.2'),
                    fontsize=FONTSIZE_MEAN_DIFF)

    # Annotate the beta binomial distribution with the alpha and beta values
    if plot_type != DIFFERENCES_ONLY_HA:
        if n == 4:
            x_coord = 0.55
            y_coord = 0.55
        elif n == 6:
            x_coord = 0.7
            y_coord = 0.48
        elif n == 8:
            x_coord = 0.65
            y_coord = 0.45
        else:
            x_coord = 0.67
            y_coord = 0.44
        subplot.annotate(rf"$\alpha$: {alpha:.2f}{chr(10)}$\beta$: {beta:.2f}",
                        xy=(mean_diff, 0),
                        xytext=(x_coord, y_coord),
                        xycoords='axes fraction',
                        ha='right',
                        fontweight='bold',
                        fontsize=FONTSIZE_SMALL)

    # Annotation with a small title box
    text = rf"$n_{{teams}} = {n}$"
    subplot.annotate(text,
                    xy=(0, 0),
                    xytext=(0.025, 0.88),
                    xycoords='axes fraction',
                    ha='left',
                    fontweight='bold',
                    bbox=dict(facecolor="#CACACA", edgecolor='black', boxstyle='round,pad=0.3'),
                    fontsize=FONTSIZE_SMALL)


def fit_curves(x_axis, diffs, max_diff, min_diff, freqs, subplot, plot_type):
    
    # Fit and plot a beta binomial distribution to the dist of diffs
    if plot_type != DIFFERENCES_ONLY_HA:
        alpha, beta = fit_beta_binom(x_axis, np.append(freqs, [0]), max_diff)
        mean_diff = betabinom.mean(max_diff, alpha, beta, loc=0)
        std_diff = betabinom.std(max_diff, alpha, beta, loc=0)
        pmf_fitted = betabinom.pmf(x_axis, max_diff, alpha, beta, loc=0)
        subplot.plot(x_axis, pmf_fitted, color='black', linestyle='-', linewidth=2)
    else:
        # Setting it in case plot_type is DIFFERENCES_ONLY_HA
        alpha, beta = 0, 0

        # For the teamless case, we use a normal distribution fit
        x_axis_precise = np.arange(min_diff, max_diff + 2, 0.1)
        mu, std = norm.fit(diffs)
        mean_diff = mu
        std_diff = std
        pdf_fitted = norm.pdf(x_axis_precise, mu, std)
        subplot.plot(x_axis_precise, pdf_fitted, color='black', linestyle='-', linewidth=2)

    return mean_diff, std_diff, alpha, beta


def make_subplot(subplot, diffs, n, plot_type, printStats=False):
    # Twinx axis for the subplot which is used for the beta binomial distribution fit
    subplot2 = subplot.twinx()

    # Set the min and max of the differences
    min_diff = np.min(diffs)
    max_found_diff = np.max(diffs)
    max_diff = n * (n - 1) * 2 - n

    # Setting it in case plot_type is DIFFERENCES_ONLY_HA
    alpha, beta = 0, 0

    # Create histogram
    x_axis = np.arange(min_diff, max_diff + 3, 2)
    freqs, _, _ = subplot.hist(diffs, bins=x_axis, align='left', color='orange', alpha=0.9, edgecolor='black', linewidth=1)
    max_y_axis = max(freqs) * 1.1

    if printStats:
        print(f"n: {n}\tMin Diff: {min_diff} \t freq: {freqs[0]}\n\tMax: {max_found_diff}\t freq: {freqs[-1]}")

    # Fit the curves based on the plot type
    mean_diff, std_diff, alpha, beta = fit_curves(x_axis, diffs, max_diff, min_diff, freqs, subplot2, plot_type)

    # Annotate plot with max, mean, and beta binomial stats
    annotate_plot(subplot, n, plot_type, max_y_axis, max_diff, mean_diff, alpha, beta)

    # Set the x-axis ticks
    if (n < 10 and plot_type != DIFFERENCES_ONLY_HA) or (n < 8 and plot_type == DIFFERENCES_ONLY_HA):
        x_values = x_axis[::2]
    elif n == 10 and plot_type == DIFFERENCES_ONLY_HA:
        # For n=10 in the teamless case, we use every 4th tick to avoid clutter
        x_values = x_axis[::6]
    else:
        # For larger n, we reduce the number of ticks to avoid clutter
        x_values = x_axis[::4]

    # Axis settings and labels
    subplot.set_xticks(x_values)
    subplot.grid(alpha=0.5)
    subplot.tick_params(axis='both', which='major', labelsize=FONTSIZE_AXIS)
    subplot.get_yaxis().get_offset_text().set_fontsize(FONTSIZE_AXIS)
    subplot2.tick_params(axis='both', which='major', labelsize=FONTSIZE_AXIS)
    subplot2.yaxis.set_major_formatter(plt.FuncFormatter(lambda val, _: f"{val:.2f}"))
    subplot.set_ylim(bottom=0, top=max_y_axis)
    subplot2.set_ylim(bottom=0)

    if printStats:
        print(f"\tMean: {mean_diff:.2f}\tStd: {std_diff:.2f}\tAlpha: {alpha:.2f}\tBeta: {beta:.2f}\n")


# Adds axis settings and labels, and saves the figure
def finish_plot(fig, file_name, show=False, twoXtwo=False):
    fig.supxlabel("Differences", fontsize=FONTSIZE)
    fig.text(-0.01, 0.5, "Frequency", va='center', ha='center', rotation=90, fontsize=FONTSIZE)  # Left side label
    fig.text(1.02, 0.5, "Probability density", va='center', ha='center', rotation=270, fontsize=FONTSIZE)  # Right side label

    # file_name += " 4x1" if not twoXtwo else ""

    # plt.xticks(np.arange(0, 21, 2))
    plt.tight_layout()
    plt.savefig(file_name, bbox_inches='tight')

    if show:
        plt.show()


def plot_diffs(files, ns, plot_type, file_name, show=False, printStats=False, twoXtwo=False):
    differences = []

    if printStats:
        if plot_type == DIFFERENCES:
            print("Plotting differences for all matchups")
        elif plot_type == DIFFERENCES_WITHOUT_HA:
            print("Plotting differences for all matchups without home/away")
        elif plot_type == DIFFERENCES_ONLY_HA:
            print("Plotting differences for home/away matchups ONLY")

    for i, file in enumerate(files):
        diff = np.array([int(diff) for diff in open(file, "r").read()[:-1].split(",")])
        if plot_type != DIFFERENCES_ONLY_HA:
            diff *= 2
        differences.append(diff)

    # Create the plot and histogram
    if twoXtwo:
        fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(24, 12), sharex=False)
    else:
        fig, axes = plt.subplots(nrows=4, ncols=1, figsize=(12, 24), sharex=False)
    fig.subplots_adjust(right=0.8)

    for i, diffs in enumerate(differences):
        if twoXtwo:
            make_subplot(axes[i // 2, i % 2], diffs, ns[i], plot_type, printStats)
        else:
            make_subplot(axes[i], diffs, ns[i], plot_type, printStats)

    # Add axis labels, layout and save the figure
    finish_plot(fig, file_name, show, twoXtwo)


if __name__ == "__main__":
    files_distances = []
    files_distances_without_ha = []
    files_distances_only_ha = []

    ns = np.arange(4, 11, 2)

    for n in ns:
        if n == 4:
            files_distances.append(f"./Distances/Distances All-{n}.csv")
            files_distances_without_ha.append(f"./Distances/Distances Reduced All-{n}.csv")
            files_distances_only_ha.append(f"./Distances/Distances Teamless All-{n}.csv")
        elif n == 6:
            files_distances.append(f"./Distances/Distances Uniform-6.csv")
            files_distances_without_ha.append(f"./Distances/Distances Reduced Uniform-6.csv")
            files_distances_only_ha.append(f"./Distances/Distances Teamless Uniform-6.csv")
        else:
            files_distances.append(f"./Distances/Distances Random-10k-{n}.csv")
            files_distances_without_ha.append(f"./Distances/Distances Reduced Random-10k-{n}.csv")
            files_distances_only_ha.append(f"./Distances/Distances Teamless Random-10k-{n}.csv")

    #plot_diffs(files_distances, ns, DIFFERENCES, file_name="Plots/Differences_n=4-10.png", show=False, printStats=True, twoXtwo=True)
    #plot_diffs(files_distances_without_ha, ns, DIFFERENCES_WITHOUT_HA, file_name="Plots/Differences_withoutHA_n=4-10.png", show=False, printStats=True, twoXtwo=True)
    plot_diffs(files_distances_only_ha, ns, DIFFERENCES_ONLY_HA, file_name="Plots/Differences_onlyHA_n=4-10.png", show=False, printStats=False, twoXtwo=True)