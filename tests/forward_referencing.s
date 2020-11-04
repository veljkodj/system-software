.section s:

mov %r0, a
jmp b
mov c(%pc), 0x10
mov (%r3), d

a: .word 0xAA
b: .word 0xBB
c: .word 0xCC
d: .word 0xDD

.end
