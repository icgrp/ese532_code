win_size = 16
prime = 3
modulus = 256
target = 0


def hash_func(input, pos):
    hash = 0
    for i in range(0, win_size):
        hash += ord(input[pos+win_size-1-i])*(pow(prime, i+1))
    return hash


def cdc(buff, buff_size):
    for i in range(win_size, buff_size-win_size):
        if((hash_func(buff, i) % modulus)) == target:
            print(i)


def test_cdc(filename):
    with open(filename) as f:
        buff = f.read()
        cdc(buff, len(buff))


test_cdc("prince.txt")
