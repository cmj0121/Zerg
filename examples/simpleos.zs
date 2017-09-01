#! /usr/bin/env zasm
#
# Simple OS designed by Zasm

__boot_loader__:	@real
	mov	ax	0x07C0	# set data segment
	mov	ds	ax

	mov	si	&BANNER

	mov	ah	0x0E	# teletype Output
	mov	bh	0x00	# page Number
	mov	bl	0x00	# color

_print_:
	lodsb	al		# load a CHAR
	cmp		al	0x0
	je		&_end_
	int 	0x10	# interrupt
	jmp		&_print_

_end_:
	jmp	$

define	BANNER		'Simple OS (boot loader) on Zasm'

repeat	0x00		$~510
repeat	'\x55\xAA'	1
