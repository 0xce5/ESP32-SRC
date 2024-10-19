import random


def generate_password():
    char_length = 600
    asciiForm = []
    for i in range(char_length):
        asciiForm.append(chr(random.randint(33, 126)))

    password = "".join(asciiForm)
    print(password)
    
generate_password()