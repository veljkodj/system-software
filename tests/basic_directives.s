.section data:

label0: .byte 0x78
label1: .byte 0x34
label2: .byte 0x11
label3: .byte 0x30
label4: .byte 0x70
label5: .byte 0x13
label6: .byte 0x88
label7: .byte 0x76

.skip 0x4

.byte 0, 1, 2, 3, 4, 5, 6, 7
.byte 0xF, 0xE, 0xD, 0xF, 0x1, 0x3, 0x99, 0x14
.byte label0, label1

.skip 8

.word 9, 10, 9, -1, 17, 4, 0xFF, 0xE
.word 0xF0, 0xCE, 0xDE, 0xFC, 0x17, 0x23, 0x19, 0x76
.word label7, label6

.skip 0xA

.end