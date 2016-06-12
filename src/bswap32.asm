page    ,132

title ByteSwap32    (bswap32.asm)

;
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
;

.586
;.mmx
.model flat, stdcall
option casemap : none

.xlist
include base64_fast.inc
.list

.stack

.data

;.data ?

.const

page

.code

assume  ds : FLAT
assume  es : FLAT
assume  ss : FLAT

align   16

public          __byteswap32

__byteswap32    proc    val:dword

OPTION PROLOGUE:NONE, EPILOGUE:NONE

                mov     eax, val
                bswap   eax
                ret

__byteswap32    endp

end
