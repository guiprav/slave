#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define fread_auto(f, ptr, n) (fread(ptr, sizeof(*ptr), n, f))

#define fwrite_auto(f, ptr, n) (fwrite(ptr, sizeof(*ptr), n, f))

int add(int a, int b) {
    return a + b;
}

void slave_exec_add() {
    int a;
    assert(fread_auto(stdin, &a, 1) == 1);

    int b;
    assert(fread_auto(stdin, &b, 1) == 1);

    int result = add(a, b);

    assert(fwrite_auto(stdout, &result, 1) == 1);
}

int multiply(int a, int b) {
    return a * b;
}

void slave_exec_multiply() {
    int a;
    assert(fread_auto(stdin, &a, 1) == 1);

    int b;
    assert(fread_auto(stdin, &b, 1) == 1);

    int result = multiply(a, b);

    assert(fwrite_auto(stdout, &result, 1) == 1);
}

enum slave_command_ids {
    slave_command_add,
    slave_command_multiply
};

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

void slave_exec(int command_id) {
    switch(command_id) {
        case slave_command_add:
            slave_exec_add();
            break;

        case slave_command_multiply:
            slave_exec_multiply();
            break;

        default:
            assert(0);
            break;
    }
}

void slave_read() {
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
}

int main() {
    while(1) {
        slave_read();
    }
}
