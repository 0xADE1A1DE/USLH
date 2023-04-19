#!/usr/bin/env python3
# run_E1.py

import matplotlib.pyplot as plt
import subprocess
import re

NO_REPETITIONS = 10000

def collect_data(argv):
    results = []
    for i in range(NO_REPETITIONS):
        proc = subprocess.Popen(argv, stdout=subprocess.PIPE)
        out, _ = proc.communicate()
        value = int(re.match(r".* (\d+)$", out.decode("utf-8").strip()).group(1))
        results.append(value)
    return results

def plot(prog):
    data_0 = collect_data([prog, "0"])
    data_1 = collect_data([prog, "1"])
    
    plt.clf()
    plt.title(prog)
    plt.hist(data_0, alpha=0.5, bins=1000, range=(0, 1000), label="0")
    plt.hist(data_1, alpha=0.5, bins=1000, range=(0, 1000), label="1")
    plt.legend()
    plt.show()

plot("./leak")
plot("./fix")
