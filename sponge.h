#ifndef SPONGE_H
#define SPONGE_H

static const char EXCEPTIONS[] = {'C', 'I', 'l', 'K', 'o', 'S',
                                  'u', 'V', 'W', 'X', 'Z'};
static const char NOT_EXCEPT[] = {'c', 'i', 'L', 'k', 'O', 's',
                                  'U', 'v', 'w', 'x', 'z'};
static constexpr int REN = static_cast<int>(sizeof(EXCEPTIONS));

/* a disgustingly simple state machine with two states
 * 1. upper case
 * 2. lower case
 * the transition is defined by whether or not the total number of upper case
 * characters is less than the total number of lower case characters.
 */
struct State_Machine {
    int consec_up;
    int consec_down;
    State_Machine(void) : consec_up(0), consec_down(0) {}

    bool next_is_uppercase(void) {
        return (consec_up < consec_down) ? true : false;
    }
};

#endif
