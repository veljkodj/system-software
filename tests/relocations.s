.section text0:
mov %r0, d
jmp d
mov c(%pc), 0x10
mov (%r3), d
a: .word 0xAA
b: .word 0xBB
.end

.section text1:
c: .word 0xCC
d: .word 0xDD
mov %r0, a
jmp b
mov a(%pc), 0x10
mov (%r3), b
.end
