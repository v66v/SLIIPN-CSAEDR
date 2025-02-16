import pandas as pd
import struct
import hashlib
import os
import argparse
from math import modf
import numpy as np
import math

csv_cols = ((2, "gas_pedal_position"),
            (3, "brake_on_off"),
            (5, "speed_vehicle_composite"),
            (6, "longitudinal_accel"),
            (7, "lateral_accel"))

DEC_LENGTH = 4
HASH_LENGTH = 8


def hash(string):
    s = hashlib.sha3_256()
    s.update(string.encode('ascii'))
    return s.hexdigest()[:HASH_LENGTH]


def canonicalize_path(string):
    return string.replace('/', '-').replace('_-_', '_')


def rotate(s, r):
    return (s * 3)[len(s) + r: 2 * len(s) + r]


def to_hex_string(fstring):
    # max 32767.9999, min -32767.9999
    if (fstring == '.'):
        fstring = '0.0'

    fstring = float(fstring)

    mod = modf(fstring)
    spl = [None, None]
    spli = [int(mod[1]), abs(mod[0])]
    # ensure valid values
    if spli[0] > 32767:
        spl[0] = "32767"
    elif spli[0] < -32767:
        spl[0] = "-32767"
    else:
        spl[0] = str(spli[0])

    if spli[1] > 0.9999:
        spl[1] = "9999"
    else:
        spl[1] = str(spli[1]).split('.')[1][:4]

    Hex = ['{0:016b}'.format(abs(int(spl[0])))[:16],
           f"{int(spl[1]):#0{6}x}"[2:]]
    # use lsb as sign
    if int(spl[0]) < 0:  # negative number
        Hex[0] = hex(int(rotate(Hex[0], 1)[:-1] + "1", 2))[2:]
    else:  # positive number or zero
        Hex[0] = hex(int(rotate(Hex[0], 1)[:-1] + "0", 2))[2:]
    # pad with zeros
    length = len(Hex[0])
    if length < DEC_LENGTH:
        Hex[0] = "0" * (DEC_LENGTH - length) + Hex[0]
    Hex = ''.join(Hex)
    return Hex


def to_float_from_hex_string(f):
    dec, =  struct.unpack('f', struct.pack('i', int(f, 16)))
    return dec


def parse_args():
    parser = argparse.ArgumentParser(
        prog='extractor',
        description='Extract columns from csv',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('filename', type=str, help='file path of csv')
    parser.add_argument('--output', nargs='?',
                        default="data/csv",
                        help='output dir [relative to cwd]')
    return parser.parse_args()


warm_up_secs = 20
warm_up_time = math.ceil(warm_up_secs / 0.1)
cool_down_secs = 20
cool_down_time = math.ceil(cool_down_secs / 0.1)

if __name__ == "__main__":
    args = parse_args()
    basename = os.path.splitext(os.path.basename(args.filename))[0]
    dir = f"{os.getcwd()}/{args.output}/{basename}"

    os.makedirs(dir, exist_ok=True)
    df = pd.read_csv(args.filename, index_col=0, dtype=str)

    for pid, cols in csv_cols:
        nlist = df[cols].replace(',', '.', regex=True).to_list()

        # n = pd.DataFrame(nlist, columns=[cols])
        first_elem = 0.0 if nlist[0] == '.' else float(nlist[0])
        last_elem = 0.0 if nlist[-1] == '.' else float(nlist[-1])

        # if pid != 3:
        #     lin = np.linspace(0, first_elem, warm_up_time).tolist()
        #     n = pd.DataFrame(lin+nlist, columns=[cols])
        # else:
        n = pd.DataFrame([first_elem] * warm_up_time +
                         nlist + [last_elem] * cool_down_time, columns=[cols])

        copy = n[cols]
        n[cols] = n[cols].apply(lambda x: to_hex_string(str(x)))

        n["pid"] = ["{:03d}".format(pid)] * len(n[cols].values)
        n["data"] = n['pid'] + "#" + n[cols]
        n["data_bare"] = n['pid'] + n[cols]

        n["sha_3"] = [hash(x) for x in n["data_bare"].values]
        n["payload"] = n["data"] + n["sha_3"]

        n.to_csv(f"{dir}/{canonicalize_path(cols)}.csv", header=None,
                 index=None, sep=' ', mode='w', columns=("payload",))

        n["payload"] = copy
        n.to_csv(f"{dir}/{canonicalize_path(cols)}-non_crypto.csv",
                 header=None, index=None, sep=' ', mode='w',
                 columns=("payload",))
