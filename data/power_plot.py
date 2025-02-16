#!/usr/bin/env python3

import sys
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('GTK3Agg')  # or 'GTK3Cairo'


def main():
    if sys.stdin.isatty():
        print("Please use a pipe as stdin\n")
        return 0

    plt.ion()
    x = []
    y = []
    fig = plt.figure()
    ax = fig.add_subplot(111)
    plt.title("Raspberry Power Consumption")
    plt.xlabel("time (sec)")
    plt.ylabel("Consumption (Watt)")
    line1, = ax.plot(x, y)

    while True:
        line = sys.stdin.readline()
        if line == '':
            break
        number = float(line)
        y.append(number)
        if not x:
            x.append(1)
        else:
            x.append(x[-1]+1)
        line1.set_xdata(x)
        line1.set_ydata(y)

        fig.canvas.draw()
        ax.relim()
        ax.autoscale()
        fig.canvas.flush_events()
    return 0


if __name__ == '__main__':
    sys.exit(main())
