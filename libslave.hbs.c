#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

{{#each fns}}
    {{either result_c_type 'void'}} {{@key}}(
        {{#each args}}
            {{c_type}} {{@key}}{{if_not @last ','}}
        {{/each}}
    );
{{/each}}

#define fread_auto(f, ptr, n) (fread(ptr, sizeof(*ptr), n, f))

#define fwrite_auto(f, ptr, n) (fwrite(ptr, sizeof(*ptr), n, f))

ssize_t slave_read_cstr(char *buf, size_t buf_len, FILE *f) {
    for(size_t i = 0; i < buf_len; ++i) {
        int c = fgetc(f);

        if(c == EOF) {
            buf[i] = 0;
            return EOF;
        }

        buf[i] = c;

        if(c == 0) {
            break;
        }
    }

    return 0;
}

#define max_string_len (512)

{{#each fns}}
    void slave_exec_{{@key}}() {
        {{#each args}}
            {{#if_cmp c_type 'eq' 'const char *'}}
                char {{@key}}[max_string_len];
                assert(slave_read_cstr({{@key}}, sizeof({{@key}}), stdin) != -1);
            {{else}}
                {{c_type}} {{@key}};
                assert(fread_auto(stdin, &{{@key}}, 1));
            {{/if_cmp}}
        {{/each}}

        {{#if result_c_type}}
            {{result_c_type}} result =
        {{/if}}
        {{@key}}(
            {{#each args}}
                {{@key}}{{if_not @last ','}}
            {{/each}}
        );

        {{#if result_c_type}}
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

int slave_dismissed() {
    return feof(stdin);
}

void slave_update() {
    slave_set_blocking_io(stdin, 0);

    int ret;

    int command_id;
    ret = fread_auto(stdin, &command_id, 1);

    if(feof(stdin)) {
        return;
    }

    if(ret <= 0 && errno == EWOULDBLOCK) {
        return;
    }

    assert(ret == 1);

    slave_set_blocking_io(stdin, 1);
    slave_exec(command_id);

    fflush(0);
}
