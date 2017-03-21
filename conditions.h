#ifndef CONDITIONS_H
#define CONDITIONS_H

const char* condition_to_string(unsigned int cond);
bool condition_is_true (struct state* s, unsigned int cond);

#endif
