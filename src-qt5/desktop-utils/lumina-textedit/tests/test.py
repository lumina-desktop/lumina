import math

primes = [2]
print(2, end=' ')
count = 1;
for i in range(3, 200000):
    for j in primes:
        if i % j == 0:
            break
        elif j > math.sqrt(i):
            count += 1
            print(i, end=' ')
            if count % 5 == 0:
                print()
            primes.append(i)
            break
print()
