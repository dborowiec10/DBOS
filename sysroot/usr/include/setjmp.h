#ifndef SETJMP_H
#define SETJMP_H

typedef int jmp_buf[18];

void	longjmp (jmp_buf __jmpb, int __retval);

int	setjmp (jmp_buf __jmpb);

#endif
