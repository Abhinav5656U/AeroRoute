# COA CO2: x86-64 GNU assembler routine for grid displacement.
# Windows x64 calling convention:
#   RCX=fromRow, RDX=fromCol, R8=toRow, R9=toCol
#   [RSP+40]=deltaX*, [RSP+48]=deltaY*, [RSP+56]=distance*

    .text
    .globl aeroroute_displacement_asm
    .def aeroroute_displacement_asm; .scl 2; .type 32; .endef
aeroroute_displacement_asm:
    # deltaX = toCol - fromCol
    movslq  %r9d, %rax
    movslq  %edx, %r10
    subq    %r10, %rax
    movq    40(%rsp), %r11
    movq    %rax, (%r11)
    movq    %rax, %r10
    imulq   %r10, %r10

    # deltaY = toRow - fromRow
    movslq  %r8d, %rax
    movslq  %ecx, %r11
    subq    %r11, %rax
    movq    48(%rsp), %r11
    movq    %rax, (%r11)
    imulq   %rax, %rax

    # distance = sqrt(deltaX^2 + deltaY^2)
    addq    %r10, %rax
    cvtsi2sdq %rax, %xmm0
    sqrtsd  %xmm0, %xmm0
    movq    56(%rsp), %rax
    movsd   %xmm0, (%rax)
    ret
