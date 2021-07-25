	.section .text_startup, "ax"
	.type start, function
	.global start
start:
	/* TODO: save memory_size params? */
	moveq #0, %d0
	move.l %d0, -(%a7)  /* argv[1] */
	move.l %a7, %d0     /* hold &argv[1] */
	move.l 12(%a3), %d1 /* module name pointer */
	move.l %d1, -(%a7)  /* argv[0] = module name */
	move.l %d0, -(%a7)  /* envp */
	subq.l #4, %d0
	move.l %d0, -(%a7)  /* argv */
	moveq #1, %d0
	move.l %d0, -(%a7)  /* argc */
	bsr.w main

	move.l %d0, %d1 /* return of main() to F$Exit status code */
exitloop:
	trap #0
	.word 6 /* F$Exit */

	bra.s exitloop
