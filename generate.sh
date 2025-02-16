#!/bin/sh

can="vcan0"
echo "Extracting datasets"
python3 data/hcar_extract.py etc/Reconstruct100CarNDSData/RawData/HundredCar_Crash_Public_Compiled.txt hcar.csv

echo "Extracting data types"
parallel python3 data/extractor.py {} ::: ./data/raw/*

echo "Randomizing order"
parallel python3 data/randomize-logger.py {}/  --can $can --output "log/random-{/}.log" ::: ./data/csv/*
