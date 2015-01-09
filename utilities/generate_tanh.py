import math

tanh_values = []

val = -5.1

for i in range(101):
    val += 0.1
    if i == 50:
        val = 0
        
    tanh_values.append(math.tanh(val))

print "{",
for val in tanh_values:
    print val, ","
print "};"
