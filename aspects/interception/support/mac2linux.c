#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>

// Event `value`; see: https://www.kernel.org/doc/html/latest/input/event-codes.html

#define DOWN 1
#define UP 0
#define REPEAT 2

// When pushing keys in the Colemak layout, these are the hardware keys seen by
// the kernel.

#define COLEMAK_A KEY_A
#define COLEMAK_C KEY_C
#define COLEMAK_EQUAL KEY_EQUAL
#define COLEMAK_F KEY_E
#define COLEMAK_G KEY_T
#define COLEMAK_L KEY_U
#define COLEMAK_MINUS KEY_MINUS
#define COLEMAK_N KEY_J
#define COLEMAK_R KEY_S
#define COLEMAK_T KEY_F
#define COLEMAK_W KEY_W
#define COLEMAK_Z KEY_Z

const struct input_event
    // TODO: deal with right alt as well
    alt_down = {.type = EV_KEY, .code = KEY_LEFTALT, .value = DOWN},
    alt_up = {.type = EV_KEY, .code = KEY_LEFTALT, .value = UP},
    alt_repeat = {.type = EV_KEY, .code = KEY_LEFTALT, .value = REPEAT},
    ctrl_down = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = DOWN},
    ctrl_up = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = UP},
    ctrl_repeat = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = REPEAT},
    syn = {.type = EV_SYN, .code = SYN_REPORT, .value = 0}
    ;

int eq(const struct input_event *a, const struct input_event *b) {
    return a->type == b->type && a->code == b->code && a->value == b->value;
}

void write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1) {
        exit(EXIT_FAILURE);
    }
}

void write_syn() {
    write_event(&syn);
    usleep(20000);
}

int main(void) {
    struct input_event event;

    enum {
        ALT_IS_ALT,
        ALT_IS_CTRL,
        ALT_IS_PENDING,
        INIT,
    } state = INIT;

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    while (fread(&event, sizeof(event), 1, stdin) == 1) {
        if (event.type == EV_KEY) {
            switch (state) {
                case INIT:
                    if (eq(&event, &alt_down) || eq(&event, &alt_repeat)) {
                        state = ALT_IS_PENDING;
                        continue;
                    }
                    break;

                case ALT_IS_PENDING:
                    if (eq(&event, &alt_down) || eq(&event, &alt_repeat)) {
                        continue;
                    } else if (eq(&event, &alt_up)) {
                        state = INIT;
                    } else if (
                        event.code == COLEMAK_A ||
                        event.code == COLEMAK_C ||
                        event.code == COLEMAK_EQUAL ||
                        event.code == COLEMAK_F ||
                        event.code == COLEMAK_G ||
                        event.code == COLEMAK_L ||
                        event.code == COLEMAK_MINUS ||
                        event.code == COLEMAK_N ||
                        event.code == COLEMAK_R ||
                        event.code == COLEMAK_T ||
                        event.code == COLEMAK_W ||
                        event.code == COLEMAK_Z
                    ) {
                        write_event(&ctrl_down);
                        write_syn();
                        state = ALT_IS_CTRL;
                    } else {
                        write_event(&alt_down);
                        write_syn();
                        state = ALT_IS_ALT;
                    }
                    break;

                case ALT_IS_ALT:
                    if (eq(&event, &alt_down) || eq(&event, &alt_repeat)) {
                        continue;
                    } else if (eq(&event, &alt_up)) {
                        state = INIT;
                    } else if (
                        event.code == COLEMAK_A ||
                        event.code == COLEMAK_C ||
                        event.code == COLEMAK_EQUAL ||
                        event.code == COLEMAK_F ||
                        event.code == COLEMAK_G ||
                        event.code == COLEMAK_L ||
                        event.code == COLEMAK_MINUS ||
                        event.code == COLEMAK_N ||
                        event.code == COLEMAK_R ||
                        event.code == COLEMAK_T ||
                        event.code == COLEMAK_W ||
                        event.code == COLEMAK_Z
                    ) {
                        write_event(&alt_up);
                        write_syn();
                        write_event(&ctrl_down);
                        write_syn();
                        state = ALT_IS_CTRL;
                    }
                    break;

                case ALT_IS_CTRL:
                    if (eq(&event, &alt_down) || eq(&event, &alt_repeat)) {
                        continue;
                    } else if (eq(&event, &alt_up)) {
                        write_event(&ctrl_up);
                        write_syn();
                        state = INIT;
                        continue;
                    } else if (
                        event.code == COLEMAK_A ||
                        event.code == COLEMAK_C ||
                        event.code == COLEMAK_EQUAL ||
                        event.code == COLEMAK_F ||
                        event.code == COLEMAK_G ||
                        event.code == COLEMAK_L ||
                        event.code == COLEMAK_MINUS ||
                        event.code == COLEMAK_N ||
                        event.code == COLEMAK_R ||
                        event.code == COLEMAK_T ||
                        event.code == COLEMAK_W ||
                        event.code == COLEMAK_Z
                    ) {
                        break;
                    } else {
                        write_event(&ctrl_up);
                        write_syn();
                        write_event(&alt_down);
                        write_syn();
                        state = ALT_IS_PENDING;
                    }
                    break;
            }
        }

        write_event(&event);
    }
}
