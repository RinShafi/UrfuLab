# 1 задача
def count_options(n, a, b):
    count = 0
    for i in range(n // a + 1):
        if (n - i * a) % b == 0:
            count += 1
    return count

# print(count_options(3, 2, 1))
# print(count_options(53, 2, 2))

# 2 задача
def check(input_str, check_str):
    i = 0
    j = 0
    while i < len(input_str) and j < len(check_str):
        if check_str[j] not in input_str:
            return False
        if input_str[i] == check_str[j]:
            i += 1
            j += 1
        else:
            j += 1
    res = i == len(input_str)
    return res

print(check("toha", "tooooooha"))

