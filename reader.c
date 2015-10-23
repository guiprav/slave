#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define max_line_len (512)
#define command_name_max_len (128)

const char *strskipspaces(const char *str) {
    const char *result = str;

    while(isspace(*result)) {
        ++result;
    }

    return result;
}

int pipedream_parse_int_arg(const char **line_args) {
    *line_args = strskipspaces(*line_args);

    assert((*line_args)[0] != 0);

    int value;

    value = strtol(*line_args, (char **)(line_args), 10);

    *line_args = strskipspaces(*line_args);

    if((*line_args)[0] == ',') {
        (*line_args)++;
    }

    return value;
}

struct pipedream_command {
    const char *name;
    void (*fn)(const char *line_args);
};

int add(int a, int b) {
    return a + b;
}

void pipedream_exec_add(const char *line_args) {
    const char *line_args_cursor = line_args;

    int a = pipedream_parse_int_arg(&line_args_cursor);
    int b = pipedream_parse_int_arg(&line_args_cursor);

    line_args_cursor = strskipspaces(line_args_cursor);

    assert(line_args_cursor[0] == 0);

    int result = add(a, b);

    printf("%d\n", result);
}

int multiply(int a, int b) {
    return a * b;
}

void pipedream_exec_multiply(const char *line_args) {
    const char *line_args_cursor = line_args;


    int a = pipedream_parse_int_arg(&line_args_cursor);
    int b = pipedream_parse_int_arg(&line_args_cursor);

    int result = multiply(a, b);

    printf("%d\n", result);
}

struct pipedream_command pipedream_commands[] = {
    {
        .name = "add",
        .fn = pipedream_exec_add
    },
    {
        .name = "multiply",
        .fn = pipedream_exec_multiply
    }
};

int pipedream_command_count = (
    sizeof(pipedream_commands) / sizeof(pipedream_commands[0])
);

void pipedream_init() {
    int fd = fileno(stdin);

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void substr(const char *str, size_t i, size_t n, char *buf, size_t buf_len) {
    assert(n < buf_len);

    memcpy(buf, str + i, n);

    buf[i + n] = 0;
}

void pipedream_parse_command_name(
    const char **line_cursor,
    char *command_name,
    size_t command_name_len
) {
    const char *command_name_in_line = *line_cursor;

    *line_cursor = strchrnul(*line_cursor, ':');

    substr(
        command_name_in_line,
        0, *line_cursor - command_name_in_line,
        command_name, command_name_len
    );

    if(**line_cursor != 0) {
        (*line_cursor)++;
    }
}

void pipedream_exec(const char *line) {
    const char *line_cursor = line;

    char command_name[command_name_max_len];

    pipedream_parse_command_name(&line_cursor, command_name, sizeof(command_name));

    int found = 0;

    for(int i = 0; i < pipedream_command_count; ++i) {
        if(strcmp(command_name, pipedream_commands[i].name) == 0) {
            found = 1;
            pipedream_commands[i].fn(line_cursor);

            break;
        }
    }

    assert(found);
}

void pipedream_read() {
    static char buf[max_line_len];
    static int buf_cursor = 0;

    if(!fgets(buf, max_line_len - buf_cursor, stdin)) {
        if(feof(stdin)) {
            exit(0);
        }

        return;
    }

    for(int i = buf_cursor; i < max_line_len; ++i) {
        char c = buf[i];
        char next_c;

        int last = (i == max_line_len - 1);

        if(!last) {
            next_c = buf[i + 1];
        }

        switch(c) {
            case '\n':
                // TODO: This wouldn't be necessary if I replaced fgets with a
                // custom line-aware reading function.
                assert(!last && next_c == 0);

                buf[i] = 0;

                pipedream_exec(buf);

                buf[0] = 0;
                buf_cursor = 0;

                goto loop_end;

            case 0:
                if(last) {
                    fprintf(stderr, "Line is too long.\n");
                    exit(-1);
                }

                goto loop_end;
        }
    }

    loop_end:

    buf_cursor += strlen(buf + buf_cursor);
}

int main() {
    pipedream_init();

    while(1) {
        pipedream_read();
    }
}
