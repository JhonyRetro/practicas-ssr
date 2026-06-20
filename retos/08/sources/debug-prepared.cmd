set pagination off
disassemble main
disassemble smash
break *smash
break *smash+4
break *smash+73
run
p $ebp-0x200
continue
x/100wx smash+51
continue
x/wx $esp
continue
exit
