title ByteSwap64    (bswap64.asm)

;
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
;

include base64_fast.inc
subttl  "__byteswap64"

_TEXT segment para 'CODE'

align   16

public          __byteswap64

__byteswap64	proc	frame
.endprolog

OPTION PROLOGUE:NONE, EPILOGUE:NONE

                bswap   rdi
                mov     rax, rdi
                ret

__byteswap64	endp

_TEXT ends

end
