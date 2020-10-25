import random
import sys
import re

# sys.argv[1]: random seed
# sys.argv[2]: number of comments to be generated
# sys.argv[3]: output file name

random.seed(int(sys.argv[1]))

alphabet = 'abc*/ '
arr = []
for i in range(int(sys.argv[2])):
    string = '/*'
    for j in range(20):
        string += random.choice(alphabet)
    string += '*/'
    arr.append(string)

with open(sys.argv[3], 'w') as f:
    for i in range(len(arr)):
        f.write(str(i) + '\n')
        f.write(arr[i])
        f.write('\n')



