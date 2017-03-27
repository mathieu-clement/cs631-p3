#ifndef FUNC_H
#define FUNC_H

// Assembly function signature
typedef unsigned int (*func)(unsigned int, unsigned int, unsigned int, unsigned int);

#define FUNC_DEF(name) unsigned int name (unsigned int a, unsigned int b, unsigned int c, unsigned int d)

FUNC_DEF(sum_array);
FUNC_DEF(fib_iter);
FUNC_DEF(fib_rec);
FUNC_DEF(find_max);
FUNC_DEF(find_str);


#endif
