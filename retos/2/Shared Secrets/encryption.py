
# Encrypt flag
flag = b"7A636965495E4C716E625579396978397E553A6E3B3F3C386F6F77"
enc = bytes([x ^ 10 for x in flag])
print(enc)
# Write challenge info

