#include <stdint.h>
#include "libslave.h"

uint32_t add(
        int32_t a,
        int32_t b
) {
    return a + b;
}

uint32_t multiply(
        int32_t a,
        int32_t b
) {
    return a * b;
}

int main() {
    while(!slave_dismissed()) {
        slave_update();
    }
}
