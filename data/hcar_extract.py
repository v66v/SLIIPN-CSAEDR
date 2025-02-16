import os
import csv
from pathlib import Path
import argparse


generator_counter = 0


def generate_outputfiles(outputdir, basefilename, basesuffix):
    """Open generated filenames, in outputdir using basefilename
    e.g. foobar.txt becomes foobar001.txt, foobar002.txt, etc."""

    global generator_counter
    outputbase = Path(basefilename)
    outputdir = Path(outputdir)
    generator_counter += 1
    return (outputdir / f'{outputbase.stem}-{generator_counter:03d}-{basesuffix}{outputbase.suffix}').open(mode='w+', newline='')


def split_data(filepath, basefilepath=None, basefilename=None, **kwargs):
    """Split filepath into multiple files based on blank lines."""

    inputpath = Path(filepath)
    outputdir = basefilepath or inputpath.parent
    outputname = basefilename or inputpath.name

    with inputpath.open(newline='') as inputfile:
        reader = list(csv.reader(inputfile, **kwargs))
        outputfile = generate_outputfiles(
            outputdir, outputname, reader[1][0])
        writer = csv.writer(outputfile, **kwargs)
        firstline = reader[0]
        try:
            for idx, row in enumerate(reader):
                if not any(row):  # found blank row
                    outputfile.close()
                    outputfile = generate_outputfiles(
                        outputdir, outputname, reader[idx + 1 if idx+1 < len(reader) else 0][0])
                    writer = csv.writer(outputfile, **kwargs)
                    writer.writerow(firstline)
                else:
                    writer.writerow(row)
        finally:
            if not outputfile.closed:
                data = outputfile.read()
                outputfile.close()
            if len(data) == 0:
                Path(outputfile.name).unlink()


def parse_args():
    parser = argparse.ArgumentParser(
        prog='extractor',
        description='Split data from file based on blank lines',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('filename', type=str, help='file path of file')
    parser.add_argument('outputfilename', type=str,
                        help='file path of output file')
    parser.add_argument('--output', nargs='?',
                        default="data/raw",
                        help='output dir [relative to cwd]')
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    os.makedirs(args.output, exist_ok=True)
    split_data(args.filename, args.output, args.outputfilename)
