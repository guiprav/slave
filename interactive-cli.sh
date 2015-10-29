#!/bin/bash
set -e

cmd_fifo_path="/tmp/$$.cmd"
data_fifo_path="/tmp/$$.data"

mkfifo "$cmd_fifo_path" "$data_fifo_path"

awk '
    /^add / {
        print "echo result \"$(xx -r --int32)\"" >"'"$cmd_fifo_path"'"
        system("xx --uint32 0 --int32 " $2 " --int32 " $3);
    }

    /^mul / {
        print "echo result \"$(xx -r --int32)\"" >"'"$cmd_fifo_path"'"
        system("xx --uint32 1 --int32 " $2 " --int32 " $3);
    }
' |./a.out >"$data_fifo_path" |(
    while read cmd
    do
        eval "$cmd" <"$data_fifo_path"
    done <"$cmd_fifo_path"
)
