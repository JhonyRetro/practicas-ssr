import time
import pyautogui

print("Switch to your other terminal...")
time.sleep(3)

# Generate a string containing 1,751 'e's
payload = 'e' * 1751

# Type the entire string at once
pyautogui.write(payload)

# If you also need it to press Enter immediately after:
pyautogui.press('enter')
