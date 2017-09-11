#! /usr/bin/env zasm
#
# Simple OS designed by Zasm

ENTRY	0x07C00

__boot_loader__:	@real
	mov	si	&BANNER

	mov	ah	0x0E	# teletype Output
	mov	bh	0x00	# page Number
	mov	bl	0x00	# color

_print_:
	lodsb	al		# load a CHAR
	cmp		al	0x0
	je		&_screen_with_VGA_
	int 	0x10	# interrupt
	jmp		&_print_

_screen_with_VGA_:
	mov	ax	0x0600	# clean-up screen
	mov	bx	0x0700
	mov	cx	0x00
	mov	dx	0x184F	# (80, 25)
	int 0x10		# interrupt
	mov ax	0xB800
	mov	gs	ax		# Set GS : B800

	mov word [gs:0x00] 0xA453
	mov word [gs:0x02] 0xA469
	mov word [gs:0x04] 0xA46D
	mov word [gs:0x06] 0xA470
	mov word [gs:0x08] 0xA46C
	mov word [gs:0x0A] 0xA465
	mov word [gs:0x0C] 0xA420
	mov word [gs:0x0E] 0xA44F
	mov word [gs:0x10] 0xA453
	mov word [gs:0x12] 0xA420
	mov word [gs:0x14] 0xA46F
	mov word [gs:0x16] 0xA46E
	mov word [gs:0x18] 0xA420
	mov word [gs:0x1A] 0xA45A
	mov word [gs:0x1C] 0xA461
	mov word [gs:0x1E] 0xA473
	mov word [gs:0x20] 0xA46D

	jmp	$

define	BANNER		'Simple OS (boot loader) on Zasm'

repeat	0x00		$~510
repeat	'\x55\xAA'	1
