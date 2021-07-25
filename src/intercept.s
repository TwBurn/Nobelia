	.section .text.intercept_thunk, "ax"
	.type intercept_thunk, function
	.global intercept_thunk
intercept_thunk:
	ext.l %d1
	move.l %d1, -(%a7)
	movea.l intercept_callback:w(%a6), %a0
	jsr (%a0)
	addq.l #4, %a7
	trap #0
	.word 30 /* F$RTE */
