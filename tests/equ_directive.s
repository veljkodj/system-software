.section text:

.equ l15, l14 - l6 + l9 + l7 - l4 + 1000
.equ l14, l13 - l9 + l3 - 0xC
.equ l13, l12
.equ l12, l11 - l7
.equ l11, l10 - l8 + l6
.equ l10, l9 + 0x100
.equ l9, l4 + l3 - l8 + l3 - l7
.equ l8, l7 - l5 - 2 + l0
.equ l7, l0
.equ l6, l5 + l1 + 0x800 - l2
.equ l5, l4 + l1 - l2 + l3 - l4
.equ l4, l3 + l2 - l0
.equ l3, l0 - l1 + l2
.equ l2, 78
.equ l1, 40 - 20 + 0xC
.equ l0, 0xB - 0xC + 3

.end
