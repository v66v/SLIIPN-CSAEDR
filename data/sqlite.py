import matplotlib.animation as animation
import argparse
import os
import numpy as np
import sqlite3
from datetime import datetime
from parse import parse
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('GTK3Agg')  # or 'GTK3Cairo'

format_string = "%Y-%m-%d %H:%M:%S.%f"
writer = "pillow"
interval = 30
speed = 2
plot_num = 12


def plot_static(x, y, name, xlabel, ylabel, save=False, dir='images', split=200, color=None, legend=[], legend_handle=False):
    plt.figure()
    plt.title(name)
    plt.ylabel(ylabel)
    plt.xlabel(xlabel)

    plt.plot(x[:split], y[:split], color or 'b')
    plt.plot(x[split-1:], y[split-1:], color or 'r')

    ax = plt.gca()
    if legend:
        if legend_handle:
            leg = ax.legend(legend, loc="upper right",
                            handlelength=0, handletextpad=0, fancybox=True)
            for item in leg.legend_handles:
                item.set_visible(False)
        else:
            ax.legend(legend, loc="upper right")

    if save:
        print(f"saving {name}")
        plt.savefig(f'{dir}/{name}.png', dpi=400)
        plt.savefig(f'{dir}/{name}.pgf', backend='pgf')


def plot_cusum_static(x, y, name, xlabel, ylabel, save=False, dir='images', split=200, color=None):
    plt.figure()
    plt.title(name)
    plt.ylabel(ylabel)
    plt.xlabel(xlabel)

    plt.plot(x[1][:split], y[1][:split], 'b')
    plt.plot(x[1][split-1:], y[1][split-1:], 'r')

    plt.plot(x[0], y[0], color)

    if save:
        print(f"saving {name}")
        plt.savefig(f'{dir}/{name}.png', dpi=400)
        plt.savefig(f'{dir}/{name}.pgf', backend='pgf')


def plot_cusum_anim(x, y, name, xlabel, ylabel, save, dir, split, color):
    fig, ax = plt.subplots()
    ax.set(title=name, xlabel=xlabel, ylabel=ylabel)

    during = ax.plot(x[1][0], y[1][0], 'r')[0]
    pre = ax.plot(x[1][0], y[1][0], 'b')[0]

    limit = ax.plot(x[0][0], y[0][0], color)[0]

    def update(frame):
        frame *= speed
        limit.set_xdata(x[0][:frame])
        limit.set_ydata(y[0][:frame])

        if frame <= 200:
            pre.set_xdata(x[1][:frame])
            pre.set_ydata(y[1][:frame])
            during.set_xdata([])
            during.set_ydata([])
        else:
            during.set_xdata(x[1][199:frame])
            during.set_ydata(y[1][199:frame])

        ax.relim()
        ax.autoscale_view()
        return (pre, during)

    ani = animation.FuncAnimation(
        fig=fig, func=update, frames=int(len(x[0])/speed), interval=interval)

    if save:
        ani.save(filename=f'{dir}/{name}.gif', writer=writer)
    else:
        return ani


def plot_cusum(x, y, name, xlabel, ylabel, save=False, anim=False, dir='images', split=200, color=None):
    if anim:
        return plot_cusum_anim(x, y, name, xlabel, ylabel, save, dir, split, color)
    else:
        plot_cusum_static(x, y, name, xlabel, ylabel, save, dir, split, color)


def plot_anim(x, y, name, xlabel, ylabel, save, dir, split, color, speedup, legend):
    fig, ax = plt.subplots()
    ax.set(title=name, xlabel=xlabel, ylabel=ylabel)

    during = ax.plot(x[0], y[0], color or 'r')[0]
    pre = ax.plot(x[0], y[0], color or 'b')[0]

    if legend:
        ax.legend(legend, loc="upper right")

    def update(frame):
        frame *= speed * speedup
        if frame <= 200 * speedup:
            pre.set_xdata(x[:frame])
            pre.set_ydata(y[:frame])
            during.set_xdata([])
            during.set_ydata([])
        else:
            during.set_xdata(x[199:frame])
            during.set_ydata(y[199:frame])

        ax.relim()
        ax.autoscale_view()
        return (pre, during)

    ani = animation.FuncAnimation(
        fig=fig, func=update, frames=int(len(x)/(speed*speedup)), interval=interval)

    if save:
        ani.save(filename=f'{dir}/{name}.gif', writer=writer)
    else:
        return ani


def plot(x, y, name, xlabel, ylabel, save=False, anim=False, dir='images', split=200, color=None, speedup=1, legend=[], legend_handle=False):
    if anim:
        return plot_anim(x, y, name, xlabel, ylabel, save, dir, split, color, speedup, legend)
    else:
        plot_static(x, y, name, xlabel, ylabel,
                    save, dir, split, color, legend, legend_handle)


def plot_delay(cur, start_time, name, xlabel='steps',
               ylabel='delta time (sec)', save=False, anim=False, dir='images'):
    res = cur.execute(
        "SELECT timestamp_sql,timestamp FROM DATA")
    resp_data = res.fetchall()

    timestmp_sql = [x[0] for x in resp_data]
    timestmp = [x[1] for x in resp_data]

    times_sql = [datetime.strptime(str, format_string)
                 for str in timestmp_sql]

    times = [datetime.strptime(str.rstrip(), format_string)
             for str in timestmp]

    delta = [(tsql-t).total_seconds() for tsql, t in zip(times_sql, times)]

    length = len(delta)
    time = np.linspace(start_time, start_time+0.1*length/plot_num, length)
    pre, post = plot_get_total_time(cur)
    plti = plot(time, delta, name, xlabel, ylabel, save,
                anim, dir, 200*plot_num, None, plot_num,
                [f"Pre crash: {pre}s", f"Post crash: {post}s"])
    return plti


def plot_get_total_time(cur):
    res = cur.execute(
        "SELECT msg FROM LOGS where type=32")
    parsed = parse('Crash Started {} {} {} {}', res.fetchall()[0][0])
    start = datetime.strptime(parsed[3].rstrip(), format_string)
    res = cur.execute(
        "SELECT msg FROM LOGS where type=64")
    parsed = parse('Crash Mid {} {}', res.fetchall()[0][0])
    mid = datetime.strptime(parsed[1].rstrip(), format_string)

    res = cur.execute(
        "SELECT msg FROM LOGS where type=128")
    parsed = parse('Crash Ended {} {}', res.fetchall()[0][0])
    end = datetime.strptime(parsed[1].rstrip(), format_string)
    pre = str(mid - start)
    post = str(end - mid)
    return pre, post


def plot_get_time(cur):
    res = cur.execute(
        "SELECT msg FROM LOGS where type=32")
    parsed = parse('Crash Started {} {} {} {}', res.fetchall()[0][0])
    row = parsed[0]
    h_h = parsed[1]
    h_l = parsed[2]
    return row, h_h, h_l


def plot_data(cur, start_time, pid, name, xlabel='steps',
              ylabel='delta time (sec)', save=False, anim=False, dir='images', legend_handle=False):
    res = cur.execute(
        f"SELECT data FROM DATA where pid={pid}")
    resp_data = res.fetchall()

    data = [x[0] for x in resp_data]

    length = len(data)
    time = np.linspace(start_time, start_time+0.1*length, length)
    return plot(time, data, name, xlabel, ylabel, save, anim, dir=dir,
                legend=[
                    f"AVG: {sum(data) / len(data):.2f}"] if legend_handle else [],
                legend_handle=legend_handle)


def plot_data_acc(cur, start_time, pid, name, xlabel='steps',
                  ylabel='delta time (sec)', save=False, anim=False, dir='images'):
    res = cur.execute(
        f"SELECT data FROM DATA where pid={pid}")
    resp_data = res.fetchall()

    data = [x[0] for x in resp_data]

    length = len(data)
    time = np.linspace(start_time, start_time+0.1*length, length)
    return plot(time, data, name, xlabel, ylabel, save, anim, dir=dir)


def plot_data_cusum(cur, start_time, h, pids, name, xlabel='steps',
                    ylabel='delta time (sec)', save=False, anim=False, dir='images'):
    res = cur.execute(
        f"SELECT data FROM DATA where pid={pids[0]}")
    resp_data_h = res.fetchall()
    res = cur.execute(
        f"SELECT data FROM DATA where pid={pids[1]}")
    resp_data_l = res.fetchall()
    res = cur.execute(
        f"SELECT data FROM DATA where pid={pids[2]}")
    resp_data_std = res.fetchall()

    data_h = [x[0] for x in resp_data_h]
    data_l = [x[0] for x in resp_data_l]

    stdenv_h = [x[0]*float(h[0]) for x in resp_data_std]
    stdenv_l = [-x[0]*float(h[1]) for x in resp_data_std]

    length = len(data_h)
    time = np.linspace(start_time, start_time+0.1*length, length)

    high = plot_cusum([time, time], [stdenv_h, data_h], name + " high",
                      xlabel, ylabel, save, anim, color='g', dir=dir)

    length = len(data_l)
    time = np.linspace(start_time, start_time+0.1*length, length)
    low = plot_cusum([time, time], [stdenv_l, data_l], name + " low",
                     xlabel, ylabel, save, anim, color='g', dir=dir)
    return high, low


def parse_args():
    parser = argparse.ArgumentParser(
        prog='sqlite',
        description='plot data from sqlite3 db',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('db', nargs='?',
                        default="edr.db",
                        help='db path [relative to cwd]')
    parser.add_argument('--output', nargs='?',
                        default="./data/images",
                        help='output dir [relative to cwd]')
    parser.add_argument('--plot', default=False, action='store_true',
                        help='plot figures')
    parser.add_argument('--metrics', default=False, action='store_true',
                        help='plot CPU Usage and DB Latency')
    parser.add_argument('--save', default=False, action='store_true',
                        help='save figures')
    parser.add_argument('--anim', default=False, action='store_true',
                        help='create animation')
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    con = sqlite3.connect(args.db)
    cur = con.cursor()
    row, h_h, h_l = plot_get_time(cur)
    start_time = int(row)-20
    save = args.save
    anim = args.anim
    metrics = args.metrics
    dir = ''
    if anim:
        print("Animating")
    if save:
        print("Saving figures")
        dir = f"{args.output}"
        os.makedirs(dir, exist_ok=True)

    if not metrics:
        acc = plot_data(cur, start_time, 2, "Accelerator Pedal Value",
                        ylabel="No Units", save=save, anim=anim, dir=dir)
        b = plot_data(cur, start_time, 3, "Brake Switch Value",
                      ylabel="$0-1$", save=save, anim=anim, dir=dir)
        sp = plot_data(cur, start_time, 5, "Vehicle Speed",
                       ylabel="$km/h$", save=save, anim=anim, dir=dir)
        acc_long = plot_data(cur, start_time, 6, "Acceleration Speed Longitudinal",
                             ylabel=r"$m/s^{2}$", save=save, anim=anim, dir=dir)
        acc_lat = plot_data(cur, start_time, 7, "Acceleration Speed Lateral",
                            ylabel=r"$m/s^{2}$", save=save, anim=anim, dir=dir)
        cusum_long = plot_data_cusum(cur, start_time, [h_h, h_l], [
                                     100, 101, 102], "CUSUM Acceleration Speed Longitudinal",
                                     ylabel=r"No Units", save=save, anim=anim, dir=dir)
        cusum_lat = plot_data_cusum(cur, start_time, [h_h, h_l], [
                                   103, 104, 105], "CUSUM Acceleration Speed Lateral", ylabel=r"No Units",
                                   save=save, anim=anim, dir=dir)

    cpu = plot_data(cur, start_time, 200, "CPU Usage",
                    ylabel="$0-100$%", save=save, anim=anim, dir=dir, legend_handle=True)
    delay = plot_delay(cur, start_time, "DB Latency",
                       save=save, anim=anim, dir=dir)

    con.close()
    if not save:
        plt.show()
