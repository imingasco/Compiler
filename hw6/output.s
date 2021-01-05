.data
.data
	_g1: .word 0
	_g2: .word 0
	_g3: .word 0
	_g4: .word 0
	_g5: .space 8
	_g6: .space 8
	_g7: .space 8
	_g8: .space 8
	_g9: .space 8
	_g10: .space 8
.text
_start_MAIN:
	sd ra, 0(sp)
	sd fp, -8(sp)
	add fp, sp, -8
	add sp, sp, -16
	la ra, _frameSize_MAIN
	lw ra, 0(ra)
	sub sp, sp, ra
	sd t0, 8(sp)
	sd t1, 16(sp)
	sd t2, 24(sp)
	sd t3, 32(sp)
	sd t4, 40(sp)
	sd t5, 48(sp)
	sd t6, 56(sp)
	sd s2, 64(sp)
	sd s3, 72(sp)
	sd s4, 80(sp)
	sd s5, 88(sp)
	sd s6, 96(sp)
	sd s7, 104(sp)
	sd s8, 112(sp)
	sd s9, 120(sp)
	sd s10, 128(sp)
	sd s11, 136(sp)
	sd fp, 144(sp)
	fsw ft0, 152(sp)
	fsw ft1, 156(sp)
	fsw ft2, 160(sp)
	fsw ft3, 164(sp)
	fsw ft4, 168(sp)
	fsw ft5, 172(sp)
	fsw ft6, 176(sp)
	fsw ft7, 180(sp)
	addi t1, x0, 1
	la t0, _g1
	sw t1, 0(t0)
.data
	FC_1: .word 0
.text
	la t1, FC_1
	flw ft0, 0(t1)
	la t0, _g2
	fsw ft0, 0(t0)
	addi t1, x0, 3
	la t0, _g3
	sw t1, 0(t0)
.data
	FC_2: .word 0
.text
	la t1, FC_2
	flw ft0, 0(t1)
	la t0, _g4
	fsw ft0, 0(t0)
	addi t1, x0, 5
	la t0, _g5
	addi t3, x0, 0
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
	addi t1, x0, 55
	la t0, _g5
	addi t3, x0, 1
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
.data
	FC_3: .word 0
.text
	la t1, FC_3
	flw ft0, 0(t1)
	la t0, _g6
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
.data
	FC_4: .word 0
.text
	la t1, FC_4
	flw ft0, 0(t1)
	la t0, _g6
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
	addi t1, x0, 7
	la t0, _g7
	addi t3, x0, 0
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
	addi t1, x0, 77
	la t0, _g7
	addi t3, x0, 1
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
.data
	FC_5: .word 0
.text
	la t1, FC_5
	flw ft0, 0(t1)
	la t0, _g8
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
.data
	FC_6: .word 0
.text
	la t1, FC_6
	flw ft0, 0(t1)
	la t0, _g8
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
	addi t1, x0, 9
	la t0, _g9
	addi t3, x0, 0
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
	addi t1, x0, 99
	la t0, _g9
	addi t3, x0, 1
	slli t3, t3, 2
	add t0, t0, t3
	sw t1, 0(t0)
.data
	FC_7: .word 0
.text
	la t1, FC_7
	flw ft0, 0(t1)
	la t0, _g10
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
.data
	FC_8: .word 0
.text
	la t1, FC_8
	flw ft0, 0(t1)
	la t0, _g10
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	fsw ft0, 0(t0)
	addi t1, x0, 1
	addi t0, x0, 4
	sub t0, fp, t0
	sw t1, 0(t0)
.data
	FC_9: .word 0
.text
	la t1, FC_9
	flw ft0, 0(t1)
	addi t0, x0, 8
	sub t0, fp, t0
	fsw ft0, 0(t0)
	addi t1, x0, 3
	addi t0, x0, 12
	sub t0, fp, t0
	sw t1, 0(t0)
.data
	FC_10: .word 0
.text
	la t1, FC_10
	flw ft0, 0(t1)
	addi t0, x0, 16
	sub t0, fp, t0
	fsw ft0, 0(t0)
	addi t1, x0, 5
	addi t0, x0, 24
	addi t3, x0, 0
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
	addi t1, x0, 55
	addi t0, x0, 24
	addi t3, x0, 1
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
.data
	FC_11: .word 0
.text
	la t1, FC_11
	flw ft0, 0(t1)
	addi t0, x0, 32
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
.data
	FC_12: .word 0
.text
	la t1, FC_12
	flw ft0, 0(t1)
	addi t0, x0, 32
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
	addi t1, x0, 7
	addi t0, x0, 40
	addi t3, x0, 0
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
	addi t1, x0, 77
	addi t0, x0, 40
	addi t3, x0, 1
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
.data
	FC_13: .word 0
.text
	la t1, FC_13
	flw ft0, 0(t1)
	addi t0, x0, 48
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
.data
	FC_14: .word 0
.text
	la t1, FC_14
	flw ft0, 0(t1)
	addi t0, x0, 48
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
	addi t1, x0, 9
	addi t0, x0, 56
	addi t3, x0, 0
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
	addi t1, x0, 99
	addi t0, x0, 56
	addi t3, x0, 1
	slli t3, t3, 2
	sub t0, t0, t3
	sub t0, fp, t0
	sw t1, 0(t0)
.data
	FC_15: .word 0
.text
	la t1, FC_15
	flw ft0, 0(t1)
	addi t0, x0, 64
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
.data
	FC_16: .word 0
.text
	la t1, FC_16
	flw ft0, 0(t1)
	addi t0, x0, 64
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	fsw ft0, 0(t0)
	la t0, _g1
	lw t1, 0(t0)
	mv a0, t1
	jal _write_int
.data
	_CONSTANT_1: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_1)
	addi a0, a5, %lo(_CONSTANT_1)
	jal _write_str
	la t0, _g2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_2: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_2)
	addi a0, a5, %lo(_CONSTANT_2)
	jal _write_str
	la t0, _g3
	lw t1, 0(t0)
	mv a0, t1
	jal _write_int
.data
	_CONSTANT_3: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_3)
	addi a0, a5, %lo(_CONSTANT_3)
	jal _write_str
	la t0, _g4
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_4: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_4)
	addi a0, a5, %lo(_CONSTANT_4)
	jal _write_str
	la t0, _g5
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_5: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_5)
	addi a0, a5, %lo(_CONSTANT_5)
	jal _write_str
	la t0, _g5
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_6: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_6)
	addi a0, a5, %lo(_CONSTANT_6)
	jal _write_str
	la t0, _g6
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_7: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_7)
	addi a0, a5, %lo(_CONSTANT_7)
	jal _write_str
	la t0, _g6
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_8: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_8)
	addi a0, a5, %lo(_CONSTANT_8)
	jal _write_str
	la t0, _g7
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_9: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_9)
	addi a0, a5, %lo(_CONSTANT_9)
	jal _write_str
	la t0, _g7
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_10: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_10)
	addi a0, a5, %lo(_CONSTANT_10)
	jal _write_str
	la t0, _g8
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_11: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_11)
	addi a0, a5, %lo(_CONSTANT_11)
	jal _write_str
	la t0, _g8
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_12: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_12)
	addi a0, a5, %lo(_CONSTANT_12)
	jal _write_str
	la t0, _g9
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_13: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_13)
	addi a0, a5, %lo(_CONSTANT_13)
	jal _write_str
	la t0, _g9
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_14: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_14)
	addi a0, a5, %lo(_CONSTANT_14)
	jal _write_str
	la t0, _g10
	addi t2, x0, 0
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_15: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_15)
	addi a0, a5, %lo(_CONSTANT_15)
	jal _write_str
	la t0, _g10
	addi t2, x0, 1
	slli t2, t2, 2
	add t0, t0, t2
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_16: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_16)
	addi a0, a5, %lo(_CONSTANT_16)
	jal _write_str
	lw t0, -4(fp)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_17: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_17)
	addi a0, a5, %lo(_CONSTANT_17)
	jal _write_str
	flw ft0, -8(fp)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_18: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_18)
	addi a0, a5, %lo(_CONSTANT_18)
	jal _write_str
	lw t0, -12(fp)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_19: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_19)
	addi a0, a5, %lo(_CONSTANT_19)
	jal _write_str
	flw ft0, -16(fp)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_20: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_20)
	addi a0, a5, %lo(_CONSTANT_20)
	jal _write_str
	addi t0, x0, 24
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_21: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_21)
	addi a0, a5, %lo(_CONSTANT_21)
	jal _write_str
	addi t0, x0, 24
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_22: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_22)
	addi a0, a5, %lo(_CONSTANT_22)
	jal _write_str
	addi t0, x0, 32
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_23: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_23)
	addi a0, a5, %lo(_CONSTANT_23)
	jal _write_str
	addi t0, x0, 32
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_24: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_24)
	addi a0, a5, %lo(_CONSTANT_24)
	jal _write_str
	addi t0, x0, 40
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_25: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_25)
	addi a0, a5, %lo(_CONSTANT_25)
	jal _write_str
	addi t0, x0, 40
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_26: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_26)
	addi a0, a5, %lo(_CONSTANT_26)
	jal _write_str
	addi t0, x0, 48
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_27: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_27)
	addi a0, a5, %lo(_CONSTANT_27)
	jal _write_str
	addi t0, x0, 48
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_28: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_28)
	addi a0, a5, %lo(_CONSTANT_28)
	jal _write_str
	addi t0, x0, 56
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_29: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_29)
	addi a0, a5, %lo(_CONSTANT_29)
	jal _write_str
	addi t0, x0, 56
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	lw t0, 0(t0)
	mv a0, t0
	jal _write_int
.data
	_CONSTANT_30: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_30)
	addi a0, a5, %lo(_CONSTANT_30)
	jal _write_str
	addi t0, x0, 64
	addi t2, x0, 0
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_31: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_31)
	addi a0, a5, %lo(_CONSTANT_31)
	jal _write_str
	addi t0, x0, 64
	addi t2, x0, 1
	slli t2, t2, 2
	sub t0, t0, t2
	sub t0, fp, t0
	flw ft0, 0(t0)
	fmv.s fa0, ft0
	jal _write_float
.data
	_CONSTANT_32: .ascii "\n\000"
.text
	lui a5, %hi(_CONSTANT_32)
	addi a0, a5, %lo(_CONSTANT_32)
	jal _write_str
	addi t0, x0, 0
	mv a0, t0
	j _end_MAIN
_end_MAIN:
	ld t0, 8(sp)
	ld t1, 16(sp)
	ld t2, 24(sp)
	ld t3, 32(sp)
	ld t4, 40(sp)
	ld t5, 48(sp)
	ld t6, 56(sp)
	ld s2, 64(sp)
	ld s3, 72(sp)
	ld s4, 80(sp)
	ld s5, 88(sp)
	ld s6, 96(sp)
	ld s7, 104(sp)
	ld s8, 112(sp)
	ld s9, 120(sp)
	ld s10, 128(sp)
	ld s11, 136(sp)
	ld fp, 144(sp)
	flw ft0, 152(sp)
	flw ft1, 156(sp)
	flw ft2, 160(sp)
	flw ft3, 164(sp)
	flw ft4, 168(sp)
	flw ft5, 172(sp)
	flw ft6, 176(sp)
	flw ft7, 180(sp)
	ld ra, 8(fp)
	mv sp, fp
	add sp, sp, 8
	ld fp, 0(fp)
	jr ra
.data
	_frameSize_MAIN: .word 248
