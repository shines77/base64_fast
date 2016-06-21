page    ,132

title ByteSwap32    (bswap32.asm)

;
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
; See: https://msdn.microsoft.com/en-us/library/ss9fh0d6.aspx
;

ifndef X64
.586
;.686p
;.mmx
;.XMM
.model flat, C
option casemap : none
endif

.xlist
;include base64_fast.inc
.list

;.stack 100h

.data
; user data

;.data ?

.const

page

.code
; user code

assume  ds : FLAT
assume  es : FLAT
assume  ss : FLAT

align   16

public			__byteswap32_

__byteswap32_   proc    val:dword

;OPTION PROLOGUE:NONE, EPILOGUE:NONE

				mov     eax, val
				bswap   eax
				ret

__byteswap32_   endp

end
