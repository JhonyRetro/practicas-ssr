ct_hex = "21c1b705764e4bfdafd01e0bfdbc38d5eadf92991cdd347064e37444e517d661cea9" 
ct_bytes = bytes.fromhex(ct_hex)

def steplfsr(lfsr):
    b7 = (lfsr >> 7) & 1
    b5 = (lfsr >> 5) & 1
    b4 = (lfsr >> 4) & 1
    b3 = (lfsr >> 3) & 1
    feedback = b7 ^ b5 ^ b4 ^ b3
    return (feedback << 7) | (lfsr >> 1)

print("Iniciando ataque de fuerza bruta...\n")

for key_guess in range(256):
    lfsr = key_guess
    decrypted_output = bytearray()

    for cipher_byte in ct_bytes:
        lfsr = steplfsr(lfsr)
        ks = lfsr
        decrypted_output.append(cipher_byte ^ ks)
        
    try:
        texto_plano = decrypted_output.decode('utf-8')

        if texto_plano.isprintable():
            print(f"[*] Seed: 0x{key_guess:02x}: {texto_plano}")
            
    except UnicodeDecodeError:
        pass
