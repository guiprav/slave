#!/bin/bash
set -e

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
            print "echo \"{{result_label}} $(xx -r --{{result_type}})\"" >"'"$cmd_file_path"'"
            system("xx --uint32 @key {{#each args}}--{{type}} " ${{add @key 2}} "{{/each}}");
        }
    {{/each}}
' |./a.out >"$data_pipe_path" |(
    while read cmd
    do
        eval "$cmd" <"$data_pipe_path"
    done <"$cmd_pipe_path"
)
