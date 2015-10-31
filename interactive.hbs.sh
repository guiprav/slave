#!/bin/bash
set -e

build_path="$(dirname "${BASH_SOURCE[0]}")"

cmd_pipe_path="/tmp/$$.cmd"
data_pipe_path="/tmp/$$.data"

mkfifo "$cmd_pipe_path" "$data_pipe_path"

trap "
    rm "$cmd_pipe_path" "$data_pipe_path"
" SIGHUP SIGINT SIGTERM

chmod 600 "$cmd_pipe_path" "$data_pipe_path"

awk '
    {{#each fns}}
        /^{{name}} / {
            print "echo \"{{result_label}} $(xx -r --{{result_type}})\"" >"'"$cmd_pipe_path"'"
            system("xx --uint32 {{@key}}"{{#each args}} " --{{type}} " ${{add @key 2}}{{/each}});
            next
        }
    {{/each}}

    /^sleep / {
        system("sleep " $2);
        next
    }

    {
        print "Unknown command: " $1 >"/dev/stderr"
        exit -1
    }
' |"$build_path/slave" >"$data_pipe_path" |(
    while read cmd
    do
        eval "$cmd" <"$data_pipe_path"
    done <"$cmd_pipe_path"
)
