import matplotlib.pyplot as plt
import sys


def plot_exp1():
    with open("./data/exp1_data.csv", "r") as data:
        y_label = "Average Throughput (operations/microsec)"
        x = []
        put_throughput = []
        get_throughput = []
        scan_throughput = []

        line = data.readline().split(",")
        x_label = line[0]
        put_label = line[1]
        get_label = line[2]
        scan_label = line[3]

        line = readline().split(",")
        while line:
            x.append(float(line[0]))
            put_throughput.append(float(line[1]))
            get_throughput.append(float(line[2]))
            scan_throughput.append(float(line[3]))

            line = readline().split(",")

        fig, (ax1, ax2, ax3) = plt.subplots(3)  
        fig.set_figheight(17)

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

        fig.savefig("experiment1")


if __name__ == '__main__':
    args = sys.argv[1:]

    if argc >= 3:
        print("Usage: python plot_experiments.py <experiment_number>")
        exit()
    
    if argc > 1:
        exp = int(argv[1])

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