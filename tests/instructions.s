.global u, d, e

.section text0:

mov f, %r3

a: movw $-4,y
b: shr %r1,a
c: add (%r4),%r1h
d: movw -10(%r1),f

.section data:

x: jle 0x14
y: jmp *f
z: jne *%r1
u: jmp 0x4


.section text1:

e: push -15
f: pop %r1

.end
