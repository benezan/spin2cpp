pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_rx
	andn	dira, arg01
LR__0001
	mov	_var01, ina
	test	_var01, arg01 wz
 if_ne	jmp	#LR__0001
	mov	result1, ina
	and	result1, arg01
_rx_ret
	ret

result1
	long	0
COG_BSS_START
	fit	496
	org	COG_BSS_START
_var01
	res	1
arg01
	res	1
arg02
	res	1
	fit	496
