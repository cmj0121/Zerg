#! /usr/bin/env zasm

__main__:
	call &banner		# jump to function 	0x05

	mov rax 0x2000001	# exit				0x07
	mov rdi 0x04		# return value		0x07
	syscall				# run syscall		0x02

banner:
	mov rax 0x2000004	# sys_write			0x07
	mov rdi 0x02		# stderr			0x07
	lea rsi &Banner		# string			0x07
	mov rdx 0x35		# size of string	0x07
	syscall				# run syscall		0x02

	mov rax 0x2000004	# sys_write			0x07
	mov rdi 0x01		# stdout			0x07
	lea rsi	&str		# string for Banner	0x07
	mov rdx 0x0C		# size of string	0x07
	syscall				# run syscall		0x02
	ret					# return			0x01

define str "Hello World\n"
define Banner "Welcome to ZASM - A prototype for a simple assembler\n"
