.global setjmp
setjmp:
	push	%ebp
	mov	%esp, %ebp

	push	%edi
	mov	8 (%ebp), %edi

	mov	%eax, 0(%edi)
	mov	%ebx, 4(%edi)
	mov	%ecx, 8(%edi)
	mov	%edx, 12(%edi)
	mov	%esi, 16(%edi)

	mov	-4(%ebp),%eax
	mov	%eax, 20(%edi)

	mov	0(%ebp), %eax
	mov	%eax, 24(%edi)

	mov	%esp, %eax
	add	$12, %eax
	mov	%eax, 28(%edi)

	mov	4(%ebp), %eax
	mov	%eax, 32(%edi)

	pop	%edi
	mov	$0, %eax
	leave
	ret

.global longjmp
longjmp:
	push	%ebp
	mov	%esp, %ebp

	mov	8(%ebp), %edi	/* get jmp_buf */
	mov	12(%ebp), %eax	/* store retval in j->eax */
	test %eax, %eax
	jne	0f
	inc	%eax
0:
	mov	%eax, 0(%edi)

	mov	24(%edi), %ebp

       /*__CLI */
	mov	28(%edi), %esp

	push 32(%edi)

	mov	0(%edi), %eax
	mov	4(%edi), %ebx
	mov	8(%edi), %ecx
	mov	12(%edi), %edx
	mov	16(%edi), %esi
	mov	20(%edi), %edi
       /*__STI */

	ret
