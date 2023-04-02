import matplotlib.pyplot as plt

y_label = "Average Throughput (operations/microsec)"

with open("./data/exp1_data.txt", "r") as data:
    x = []
    put_throughput = []
    get_throughput = []
    scan_throughput = []

    for line in data.readlines():
        line = line.split(" ")
        x.append(float(line[0]))
        temp_num_puts = float(line[1].split(",")[0])
        temp_num_gets = float(line[2].split(",")[0])
        temp_num_scans = float(line[3].split(",")[0])

        temp_puts_microsec = float(line[1].split(",")[1])
        temp_gets_microsec = float(line[2].split(",")[1])
        temp_scans_microsec = float(line[3].split(",")[1])

        put_throughput.append(temp_num_puts / temp_puts_microsec)
        get_throughput.append(temp_num_gets / temp_gets_microsec)
        scan_throughput.append(temp_num_scans / temp_scans_microsec)

    fig, (ax1, ax2, ax3) = plt.subplots(3)  
    fig.set_figheight(17)

    ax1.plot(x, put_throughput)
    ax1.set_title("Put Throughput")
    ax1.set_xlabel("Inserted data (Bytes)")
    ax1.set_ylabel(y_label)

    ax2.plot(x, get_throughput)
    ax2.set_title("Get Throughput")
    ax2.set_xlabel("Inserted data (Bytes)")
    ax2.set_ylabel(y_label)

    ax3.plot(x, scan_throughput)
    ax3.set_title("Scan Throughput")
    ax3.set_xlabel("Inserted data (Bytes)")
    ax3.set_ylabel(y_label)

    fig.savefig("experiment1")