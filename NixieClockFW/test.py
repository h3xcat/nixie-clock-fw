import time
import sys
import random
start = "161234"

if __name__ == '__main__':
    c = 0
    while True:
        time.sleep(1)
        sys.stdout.write(f"\r{c:0>6}{'':<20}")
        sys.stdout.flush()
        c=random.randrange(0,20)