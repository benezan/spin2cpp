pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_index
	sub	arg02, #1
	add	arg02, arg01
	rdbyte	result1, arg02
_index_ret
	ret

result1
	long	0
COG_BSS_START
	fit	496
	org	COG_BSS_START
arg01
	res	1
arg02
	res	1
	fit	496
