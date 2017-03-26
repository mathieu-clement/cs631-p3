#ifndef ARMEMU_H
#define ARMEMU_H

#include "state.h"

void armemu_one (struct state* s);
void armemu (struct state* s);

#endif
