import matplotlib.pyplot as plt
import sys


def plot_exp1():
    with open("./experiments/data/exp1_data.csv", "r") as data:
        y_label = "Average Throughput (operations/msec)"
        x = []
        put_throughput = []
        get_throughput = []
        scan_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        put_label = line[1]
        get_label = line[2]
        scan_label = line[3]

        line = data.readline().split(",")
        while line != ['']:
            x.append(float(line[0]))
            put_throughput.append(float(line[1]))
            get_throughput.append(float(line[2]))
            scan_throughput.append(float(line[3]))

            line = data.readline().split(",")

        fig, (ax1, ax2, ax3) = plt.subplots(3)
        fig.set_figheight(20)

        ax1.plot(x, put_throughput)
        ax1.set_title(put_label)
        ax1.set_xlabel(x_label)
        ax1.set_ylabel(y_label)

        ax2.plot(x, get_throughput)
        ax2.set_title(get_label)
        ax2.set_xlabel(x_label)
        ax2.set_ylabel(y_label)

        ax3.plot(x, scan_throughput)
        ax3.set_title(scan_label)
        ax3.set_xlabel(x_label)
        ax3.set_ylabel(y_label)

        fig.savefig("experiment1.png")

def plot_exp2p1():
    with open("./experiments/data/exp2p1_data.csv", "r") as data:
        y_label = "Average Throughput (operations/msec)"
        x = []
        lru1_throughput = []
        clock1_throughput = []
        lru2_throughput = []
        clock2_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        lru1_label = line[1]
        clock1_label = line[2]
        lru2_label = line[3]
        clock2_label = line[4]

        line = data.readline().split(",")
        while line != ['']:
            x.append(float(line[0]))
            lru1_throughput.append(float(line[1]))
            clock1_throughput.append(float(line[2]))
            lru2_throughput.append(float(line[3]))
            clock2_throughput.append(float(line[4]))

            line = data.readline().split(",")

        fig, (ax1, ax2) = plt.subplots(2)
        fig.set_figheight(12)

        ax1.plot(x, lru1_throughput, label=lru1_label)
        ax1.plot(x, clock1_throughput, label=clock1_label)
git status        ax1.legend()
        ax1.set_title("Experiment 2.1 (Clock Better): LRU Buffer vs. Clock Buffer")
        ax1.set_xlabel(x_label)
        ax1.set_ylabel(y_label)

        ax2.plot(x, lru2_throughput, label=lru2_label)
        ax2.plot(x, clock2_throughput, label=clock2_label)
        ax2.legend()
        ax2.set_title("Experiment 2.1 (LRU Better): LRU Buffer vs. Clock Buffer")
        ax2.set_xlabel(x_label)
        ax2.set_ylabel(y_label)

        fig.savefig("experiment2p1.png")


def plot_exp2p2():
    with open("./experiments/data/exp2p2_data.csv", "r") as data:
        y_label = "Average Throughput (operations/msec)"
        x = []
        btree_throughput = []
        bs_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        btree_label = line[1]
        bs_label = line[2]

        line = data.readline().split(",")
        while line != ['']:
            x.append(float(line[0]))
            btree_throughput.append(float(line[1]))
            bs_throughput.append(float(line[2]))

            line = data.readline().split(",")

        plt.plot(x, btree_throughput, label=btree_label)
        plt.plot(x, bs_throughput, label=bs_label)
        plt.legend()
        plt.title("Experiment 2.2: BTree Search vs. Binary Search")
        plt.xlabel(x_label)
        plt.ylabel(y_label)

        plt.savefig("experiment2p2.png")


def plot_exp3p1():
    with open("./experiments/data/exp3p1_data.csv", "r") as data:
        y_label = "Average Throughput (operations/msec)"
        x = []
        put_throughput = []
        get_throughput = []
        scan_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        put_label = line[1]
        get_label = line[2]
        scan_label = line[3]

        line = data.readline().split(",")
        while line != ['']:
            x.append(float(line[0]))
            put_throughput.append(float(line[1]))
            get_throughput.append(float(line[2]))
            scan_throughput.append(float(line[3]))

            line = data.readline().split(",")

        fig, (ax1, ax2, ax3) = plt.subplots(3)
        fig.set_figheight(20)

        ax1.plot(x, put_throughput)
        ax1.set_title(put_label)
        ax1.set_xlabel(x_label)
        ax1.set_ylabel(y_label)

        ax2.plot(x, get_throughput)
        ax2.set_title(get_label)
        ax2.set_xlabel(x_label)
        ax2.set_ylabel(y_label)

        ax3.plot(x, scan_throughput)
        ax3.set_title(scan_label)
        ax3.set_xlabel(x_label)
        ax3.set_ylabel(y_label)

        fig.savefig("experiment3p1.png")

def plot_exp3p2():
    with open("./experiments/data/exp3p2_data.csv", "r") as data:
        x = []
        get_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        y_label = line[1]

        line = data.readline().split(",")
        while line != ['']:
            x.append(float(line[0]))
            get_throughput.append(float(line[1]))

            line = data.readline().split(",")

        plt.plot(x, get_throughput)
        plt.title("Experiment 3.2: Query Throughput as M increases")
        plt.xlabel(x_label)
        plt.ylabel(y_label)

        plt.savefig("experiment3p2.png")


if __name__ == '__main__':
    args = sys.argv[1:]

    if len(sys.argv) >= 3:
        print("Usage: python plot_experiments.py <experiment_number>")
        exit()

    if len(sys.argv) > 1:
        exp = int(sys.argv[1])

        if exp == 1:
            plot_exp1()
        if exp == 21:
            plot_exp2p1()
        if exp == 22:
            plot_exp2p2()
        if exp == 31:
            plot_exp3p1()
        if exp == 32:
            plot_exp3p2()
    else:
        plot_exp1()
        plot_exp2p1()
        plot_exp2p2()
        plot_exp3p1()
        plot_exp3p2()