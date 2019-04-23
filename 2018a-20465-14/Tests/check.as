; aaaaaaaaaaaaaaaaaaaaaa
; dsfKAFHHCIZHIVABCSBKAKLFHL46511946344893

.entry L
.entry ABC
.entry CBA
	.extern A
 .extern B
.extern C



	M: mov #-2,r0
 	mov r0,S1.1
	mov S1.1 , S2.1
cmp X	,	#4
cmp r3 , r4
 L: add #1 , S3.1
 add r1,r3
S: sub #5 ,	X
   sub X,X
   not r0
not S1.1
ABC: not X
lea S2.2 ,	S3.2
inc A
inc r7
inc S2.1
CBA: dec B
dec r7
dec S2.1
jmp C
bne r5
bne S3.2
red S2.2
prn C
prn B
prn #-7
prn S1.2
rts 
E: stop






STR1: .string "abcd"
STR2: .string "abcdef"
STR3: .string "abc"
X: .data +4
ARR: .data 	+5,-6	, 4 , +8,	 -3
S1: .struct 123 , 	"abc"
S2: .struct -98 , "acd"
S3: 		.struct 	+5,"ab"
