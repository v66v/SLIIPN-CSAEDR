#!/bin/bash

stty -echoctl

main=""
can_play=""
gtk=""
kill_all_cmds() {
    kill $main
    kill $can_play
    kill $gtk
    exit 0;
}

plot() {
  python3 data/sqlite.py $@
}

edr_file="edr-$(basename $1 .log).db"

if [ -n "$2" ]; then
    plot $edr_file ${@:2}
    exit;
fi

rm -f $edr_file

rec_can="vcan0"
send_can="vcan0"
trap 'kill_all_cmds' SIGINT

./build/bin/main $rec_can $edr_file & main=$!
./build/bin/gtk & gtk=$!
./build/bin/can_play $1 $send_can & can_play=$!

wait -n
ret=$?
echo "Exit Code: $ret"
if [ $ret -eq 0 ]; then
    wait -n
    kill $gtk
else
    kill $can_play
    kill $gtk
    kill $main
fi

trap - SIGINT

plot $edr_file ${@:2}
