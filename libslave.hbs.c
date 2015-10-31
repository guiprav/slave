#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

{{#each fns}}
    {{result_type}}_t {{@key}}(
        {{#each args}}
            {{#if is_string}}
                const char *
            {{else}}
                {{type}}_t
            {{/if}}
            {{@key}}{{if_not @last ','}}
        {{/each}}
    );
{{/each}}

#define fread_auto(f, ptr, n) (fread(ptr, sizeof(*ptr), n, f))

#define fwrite_auto(f, ptr, n) (fwrite(ptr, sizeof(*ptr), n, f))

{{#each fns}}
    void slave_exec_{{@key}}() {
        {{#each args}}
            {{#if is_string}}
                const char *{{@key}} = 0;
                size_t {{@key}}__getdelim_n = 0;

                assert(getdelim(&{{@key}}, &{{@key}}__getdelim_n, 0, stdin) != -1);
            {{else}}
                {{type}}_t {{@key}};
                assert(fread_auto(stdin, &{{@key}}, 1));
            {{/if}}
        {{/each}}

        {{#if result_type}}
            {{result_type}}_t result =
        {{/if}}
        {{@key}}(
            {{#each args}}
                {{@key}}{{if_not @last ','}}
            {{/each}}
        );

        {{#if result_type}}
            assert(fwrite_auto(stdout, &result, 1) == 1);
        {{/if}}
    }
{{/each}}

enum slave_command_ids {
    {{#each fns}}
        slave_command_{{@key}}{{if_not @last ','}}
    {{/each}}
};

void slave_exec(int command_id) {
    switch(command_id) {
        {{#each fns}}
            case slave_command_{{@key}}:
                slave_exec_{{@key}}();
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
