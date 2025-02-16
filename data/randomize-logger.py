import os
import argparse
import random


def slurp_file(filename):
    with open(filename) as fp:
        lines = fp.read().splitlines()
    return lines


def parse_args():
    parser = argparse.ArgumentParser(
        prog="randomize-logger",
        description="""Randomly select lines from files in a
                       directory add concatenate them into
                       a single file in order for canplay to replay""",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("dirname", type=str, help="directory path of " +
                        "csvs [relative to cwd]")
    parser.add_argument("--can", type=str,
                        default="vcan0", help="can name to send to")
    parser.add_argument("--output", nargs="?",
                        default="log/random.log",
                        help="output file [relative to cwd]")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    dir = f"{os.getcwd()}/{args.dirname}"
    filepath = f"{os.getcwd()}/{args.output}"
    os.makedirs(os.path.dirname(filepath), exist_ok=True)

    files = [f'{dir}{file}' for file in os.listdir(dir)]

    file_contents = []
    time = 0.0
    for file in files:
        if not file.endswith("-non_crypto.csv"):
            file_contents += [slurp_file(file)]

    lrange = list(range(len(file_contents)))
    lrange_tmp = lrange.copy()
    with open(filepath, "w") as fp:
        while lrange:
            index = random.choice(lrange_tmp)
            lrange_tmp.remove(index)

            line = file_contents[index].pop(0)
            print(f"({'{:017.6f}'.format(time)}) {args.can} {line}", file=fp)
            time += 0.01
            if not file_contents[index]:
                lrange.remove(index)

            if not lrange_tmp:
                lrange_tmp = lrange.copy()
