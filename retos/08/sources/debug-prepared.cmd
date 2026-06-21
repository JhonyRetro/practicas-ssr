set pagination off

disassemble main
disassemble smash

break *smash
break *smash+4
break *smash+74

run

print/x $ebp-0x200
continue

x/100wx $ebp-0x200
continue

x/4wx $esp
continue

quit
