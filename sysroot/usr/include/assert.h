#ifndef ASSERT_H
#define ASSERT_H

#define assert(expression)  \
  ((void)((expression) ? 0 : 1))

#endif
