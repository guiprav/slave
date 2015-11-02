#!/bin/bash
set -e

cmd_pipe_path="/tmp/$$.cmd"
data_pipe_path="/tmp/$$.data"

mkfifo "$cmd_pipe_path" "$data_pipe_path"

function cleanup() {
    rm "$cmd_pipe_path" "$data_pipe_path"
    exit
}

trap cleanup EXIT TERM

chmod 600 "$cmd_pipe_path" "$data_pipe_path"

awk '
    {{#each fns}}
        /^{{@key}}{{#if args}} {{else}}${{/if}}/ {
            {{#if result_c_type}}
                {{assert result_bin_type "Missing binary type."}}
                print "echo \"{{result_label}} $(xx -r --{{result_bin_type}})\"" >"'"$cmd_pipe_path"'"
            {{/if}}
            system("xx --uint32 {{@index}}"{{#each args}} " --{{bin_type}} " ${{add @index 2}}{{/each}});
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

    END {
        print "exit" >"'"$cmd_pipe_path"'"
    }
' |"{{slave_path}}" |tee "$data_pipe_path" >/dev/null |(
    while read cmd
    do
        eval "$cmd" <"$data_pipe_path"
    done <"$cmd_pipe_path"
)
