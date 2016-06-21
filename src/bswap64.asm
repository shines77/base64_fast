title ByteSwap64    (bswap64.asm)

;
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
; See: http://www.cppblog.com/luqingfei/archive/2010/08/11/123078.aspx
;

;include base64_fast.inc
subttl  "__byteswap64"

_TEXT segment para 'CODE'

;
; See: https://msdn.microsoft.com/en-us/library/ms235286.aspx
;
; The x64 Application Binary Interface (ABI) is a 4 register fast-call calling convention,
; with stack-backing for those registers. There is a strict one-to-one correspondence
; between arguments in a function, and the registers for those arguments.
; Any argument that doesn¡¯t fit in 8 bytes, or is not 1, 2, 4, or 8 bytes,
; must be passed by reference. There is no attempt to spread a single argument
; across multiple registers. The x87 register stack is unused. It may be used,
; but must be considered volatile across function calls. All floating point operations
; are done using the 16 XMM registers.

; The arguments are passed in registers RCX, RDX, R8, and R9. If the arguments are float/double,
; they are passed in XMM0L, XMM1L, XMM2L, and XMM3L. 16 byte arguments are passed by reference.
; Parameter passing is described in detail in Parameter Passing. In addition to these registers,
; RAX, R10, R11, XMM4, and XMM5 are volatile. All other registers are non-volatile.
;

;
; See: http://stackoverflow.com/questions/21991961/fastcall-what-happens-with-the-stack
;
; From what I currently understand from MSDN and Intel, the first 4 integer/floating point parameters
; are stored in the rcx/xmm0, rdx/xmm1, r8/xmm2 and r9/xmm3 registers and all others will be placed on the stack.
;
; Arguments: rcx, rdx, r8, r9, only 4th argument.
;

;
; See: http://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/
;
; On Linux, gcc or AMD64:
;
; According to the ABI, the first 6 integer or pointer arguments to a function are passed in registers.
; The first is placed in rdi, the second in rsi, the third in rdx, and then rcx, r8 and r9.
; Only the 7th argument and onwards are passed on the stack.
;
; Arguments: rdi, rsi, rdx, rcx, r8, r9, only 7th argument.
;

align   16

public          __byteswap64

__byteswap64    proc    frame
.endprolog

OPTION PROLOGUE:NONE, EPILOGUE:NONE

                bswap   rcx
                mov     rax, rcx
                ret

__byteswap64    endp

_TEXT ends

end
