#ifndef CONDITIONS_H
#define CONDITIONS_H

char* condition_to_str(unsigned int cond);
bool condition_is_true (struct state* s, unsigned int cond);

#endif
