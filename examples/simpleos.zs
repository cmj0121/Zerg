#! /usr/bin/env zasm
#
# Simple OS designed by Zasm

__main__:	@real

	mov	ah	0x0E	# Teletype Output
	mov	al	0x4A	# Output Character
	mov	bh	0x00	# Page Number
	mov	bl	0x00	# Color
	int 	0x10	# Interrupt

	jmp	$

repeat	0x00		$~510
repeat	'\x55\xAA'	1
