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

	sub		rsp		0x10
	mov		byte  [rsp+0x00]	0x20
	mov		word  [rsp+0x01]	0x202D
	mov		dword [rsp+0x03]	0x6D73615A
	mov		byte  [rsp+0x07]	0x0A
	mov		rax 0x2000004
	mov		rdi 0x01
	mov		rsi	rsp
	mov		rdx 0x08
	syscall
	add		rsp	0x10

	ret					# return			0x01

define str "Hello World"
define Banner "Welcome to ZASM - A prototype for a simple assembler\n"
