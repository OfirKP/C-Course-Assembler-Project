abcdefg123456789

123: .string "ABCDabcd"
STR: .string "ABCDabcd"
STR: .string "ABCD"
str: .string "ABCDabcd
     .string ABCDabcd

LENGTH: .data 6 , -5, 4 
LENGTH1: .data 6 , , 5 ,-4
LENGTH2: .data6 , 5 , 4
LENGTH: .data 6,5

K: .struct 2 , "Abc"
S1: .struct 3, ,"abc"
S2: .struct 4 , ABC
    .struct 5 "abc"

L: .entry LOOP
   .entry STR
   .ENTRY LENGTH
   .extern W
   .extern E

MAIN: mov S1.1 , W , LENGTH
      mov r9 , K.2
      mov r7
      cmp #54 , K.1 , STR
      cmp #-2 ,,, , W
      not
      lea r5,E
MAIN: sub #-2,#4
      JmP W
      red r2
      red LENGTH ,
      lea
LOOP: jmp W
      prn #-5
      prn E , W
      inc #4
END:  stop      Q       
      

