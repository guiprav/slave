#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

{{#each fns}}
    {{result_type}} {{name}}(
        {{#each args}}
            {{type}} {{name}}{{if_not @last ','}}
        {{/each}}
    );
{{/each}}

#define fread_auto(f, ptr, n) (fread(ptr, sizeof(*ptr), n, f))

#define fwrite_auto(f, ptr, n) (fwrite(ptr, sizeof(*ptr), n, f))

{{#each fns}}
    void slave_exec_{{name}}() {
        {{#each args}}
            {{type}} {{name}};
            assert(fread_auto(stdin, &{{name}}, 1));
        {{/each}}

        {{result_type}} result = {{name}}(
            {{#each args}}
                {{name}}{{if_not @last ','}}
            {{/each}}
        );

        assert(fwrite_auto(stdout, &result, 1) == 1);
    }
{{/each}}

enum slave_command_ids {
    {{#each fns}}
        slave_command_{{name}}{{if_not @last ','}}
    {{/each}}
};

void slave_exec(int command_id) {
    switch(command_id) {
        {{#each fns}}
            case slave_command_{{name}}:
                slave_exec_{{name}}();
                break;
        {{/each}}

        default:
            assert(0);
            break;
    }
}

void slave_set_blocking_io(FILE *f, int enable) {
    int fd = fileno(f);
    int flags = fcntl(fd, F_GETFL, 0);

    if(enable) {
        flags &= ~O_NONBLOCK;
    }
    else {
        flags |= O_NONBLOCK;
    }

    fcntl(fd, F_SETFL, flags);
}

void slave_update() {
    slave_set_blocking_io(stdin, 0);

    int ret;

    int command_id;
    ret = fread_auto(stdin, &command_id, 1);

    if(feof(stdin)) {
        exit(0);
    }

    if(ret <= 0 && errno == EWOULDBLOCK) {
        return;
    }

    assert(ret == 1);

    slave_set_blocking_io(stdin, 1);
    slave_exec(command_id);

    fflush(0);
}

int main() {
    while(1) {
        slave_update();
    }
}
