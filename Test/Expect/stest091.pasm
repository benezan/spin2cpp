pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_sendchar1
	add	objptr, #4
	call	#_simplepin_tx
	sub	objptr, #4
_sendchar1_ret
	ret

_sendchar2
	add	objptr, #16
	call	#_simplepin_tx
	sub	objptr, #16
_sendchar2_ret
	ret

_sendchar_index
	shl	arg01, #2
	mov	sendchar_index_tmp001_, arg01
	add	sendchar_index_tmp001_, #8
	mov	arg01, arg02
	add	objptr, sendchar_index_tmp001_
	call	#_simplepin_tx
	sub	objptr, sendchar_index_tmp001_
_sendchar_index_ret
	ret

_sendchar_abstract
	mov	_sendchar_abstract_fds, arg01
	mov	arg01, arg02
	mov	sendchar_abstract_tmp002_, objptr
	mov	objptr, _sendchar_abstract_fds
	call	#_simplepin_tx
	mov	objptr, sendchar_abstract_tmp002_
_sendchar_abstract_ret
	ret

_simplepin_tx
	mov	_var01, #1
	rdlong	_var02, objptr
	shl	_var01, _var02
	test	arg01, #1 wz
	muxnz	outa, _var01
_simplepin_tx_ret
	ret

objptr
	long	@@@objmem
COG_BSS_START
	fit	496
objmem
	long	0[6]
	org	COG_BSS_START
_sendchar_abstract_fds
	res	1
_var01
	res	1
_var02
	res	1
arg01
	res	1
arg02
	res	1
sendchar_abstract_tmp002_
	res	1
sendchar_index_tmp001_
	res	1
	fit	496
