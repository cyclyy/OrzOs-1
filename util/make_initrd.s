	.file	"make_initrd.c"
	.section	.rodata
.LC0:
	.string	"r"
.LC1:
	.string	"w"
.LC2:
	.string	"initrd.img"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$1062320, %esp
	movl	8(%ebp), %eax
	subl	$1, %eax
	movl	%eax, 13724(%esp)
	movl	13724(%esp), %eax
	imull	$137, %eax, %eax
	addl	$4, %eax
	movl	%eax, 1062312(%esp)
	movl	$0, 1062316(%esp)
	jmp	.L2
.L3:
	movl	1062316(%esp), %eax
	imull	$137, %eax, %eax
	leal	1062320(%esp), %edx
	leal	(%edx,%eax), %eax
	subl	$1062296, %eax
	movb	$-3, (%eax)
	movl	1062316(%esp), %eax
	addl	$1, %eax
	sall	$2, %eax
	addl	12(%ebp), %eax
	movl	(%eax), %eax
	leal	24(%esp), %ecx
	movl	1062316(%esp), %edx
	imull	$137, %edx, %edx
	leal	(%ecx,%edx), %edx
	addl	$1, %edx
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	strcpy
	movl	1062316(%esp), %eax
	imull	$137, %eax, %eax
	leal	1062320(%esp), %ecx
	leal	(%ecx,%eax), %eax
	leal	-1062168(%eax), %edx
	movl	1062312(%esp), %eax
	movl	%eax, 1(%edx)
	movl	$.LC0, %edx
	movl	1062316(%esp), %eax
	addl	$1, %eax
	sall	$2, %eax
	addl	12(%ebp), %eax
	movl	(%eax), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, 1062308(%esp)
	movl	$2, 8(%esp)
	movl	$0, 4(%esp)
	movl	1062308(%esp), %eax
	movl	%eax, (%esp)
	call	fseek
	movl	1062308(%esp), %eax
	movl	%eax, (%esp)
	call	ftell
	movl	1062316(%esp), %edx
	imull	$137, %edx, %edx
	leal	1062320(%esp), %ecx
	leal	(%ecx,%edx), %edx
	subl	$1062168, %edx
	movl	%eax, 5(%edx)
	movl	1062308(%esp), %eax
	movl	%eax, (%esp)
	call	fclose
	movl	1062316(%esp), %eax
	imull	$137, %eax, %eax
	leal	1062320(%esp), %edx
	leal	(%edx,%eax), %eax
	subl	$1062168, %eax
	movl	5(%eax), %eax
	addl	%eax, 1062312(%esp)
	addl	$1, 1062316(%esp)
.L2:
	movl	13724(%esp), %eax
	cmpl	%eax, 1062316(%esp)
	jl	.L3
	movl	$.LC1, %edx
	movl	$.LC2, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, 1062304(%esp)
	movl	1062304(%esp), %eax
	movl	%eax, 12(%esp)
	movl	$1, 8(%esp)
	movl	$4, 4(%esp)
	leal	13724(%esp), %eax
	movl	%eax, (%esp)
	call	fwrite
	movl	13724(%esp), %eax
	movl	1062304(%esp), %edx
	movl	%edx, 12(%esp)
	movl	%eax, 8(%esp)
	movl	$137, 4(%esp)
	leal	24(%esp), %eax
	movl	%eax, (%esp)
	call	fwrite
	movl	$0, 1062316(%esp)
	jmp	.L4
.L5:
	movl	$1048576, 8(%esp)
	movl	$0, 4(%esp)
	leal	13728(%esp), %eax
	movl	%eax, (%esp)
	call	memset
	movl	$.LC0, %edx
	movl	1062316(%esp), %eax
	addl	$1, %eax
	sall	$2, %eax
	addl	12(%ebp), %eax
	movl	(%eax), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, 1062308(%esp)
	movl	1062316(%esp), %eax
	imull	$137, %eax, %eax
	leal	1062320(%esp), %ecx
	leal	(%ecx,%eax), %eax
	subl	$1062168, %eax
	movl	5(%eax), %eax
	movl	1062308(%esp), %edx
	movl	%edx, 12(%esp)
	movl	$1, 8(%esp)
	movl	%eax, 4(%esp)
	leal	13728(%esp), %eax
	movl	%eax, (%esp)
	call	fread
	movl	1062308(%esp), %eax
	movl	%eax, (%esp)
	call	fclose
	movl	1062316(%esp), %eax
	imull	$137, %eax, %eax
	leal	1062320(%esp), %edx
	leal	(%edx,%eax), %eax
	subl	$1062168, %eax
	movl	5(%eax), %eax
	movl	1062304(%esp), %edx
	movl	%edx, 12(%esp)
	movl	$1, 8(%esp)
	movl	%eax, 4(%esp)
	leal	13728(%esp), %eax
	movl	%eax, (%esp)
	call	fwrite
	addl	$1, 1062316(%esp)
.L4:
	movl	13724(%esp), %eax
	cmpl	%eax, 1062316(%esp)
	jl	.L5
	movl	1062304(%esp), %eax
	movl	%eax, (%esp)
	call	fclose
	movl	$0, %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.5.0 20100610 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
