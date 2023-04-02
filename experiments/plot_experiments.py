import matplotlib.pyplot as plt

y_label = "Average Throughput (operations/msec)"

with open("exp1_data.txt", "r") as data:
    x = []
    puts_per_msec = []
    gets_per_msec = []
    scans_per_msec = []

    i = 0
    for line in data.readlines():
        line.split(",")
        x = line[0]
        puts_per_msec = line[1]
        gets_per_msec = line[2]
        scans_per_msec = line[3]

    plt.figure(1)
    plt.plot(x, puts_per_msec)
    plt.title("Put Throughput")
    plt.xlabel("Unique keys")
    plt.ylabel(y_label)
    plt.save("experiment1_put")
    plt.save_fig("experiment1_put")

    plt.figure(2)
    plt.plot(x, gets_per_msec)
    plt.title("Get Throughput")
    plt.xlabel("Unique keys")
    plt.ylabel(y_label)
    plt.save("experiment1_get")
    plt.save_fig("experiment1_get")

    plt.figure(3)
    plt.plot(x, scans_per_msec)
    plt.title("Scan Throughput")
    plt.xlabel("Unique keys")
    plt.ylabel(y_label)
    plt.save("experiment1_scan")
    plt.save_fig("experiment1_scan")

    plt.show()
    plt.save_fig("Experiment1.png")