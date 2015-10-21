#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define buf_len (512)

void pipedream_init() {
    int fd = fileno(stdin);

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void pipedream_exec(const char *line) {
    printf("%s\n", line);
}

void pipedream_read() {
    static char buf[buf_len];
    static int buf_cursor = 0;

    if(!fgets(buf, buf_len - buf_cursor, stdin)) {
        if(feof(stdin)) {
            exit(0);
        }

        return;
    }

    for(int i = buf_cursor; i < buf_len; ++i) {
        char c = buf[i];
        char next_c;

        int last = (i == buf_len - 1);

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
