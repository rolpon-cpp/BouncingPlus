print("\n")

import os

lines = 0

files = []

def count_lines(fp):
    f = open(fp,'r')
    l = 0
    while f.readline():
        l += 1
    return l

def cool(dir1):
    s = ""
    if "cmake" in dir1:
        return s
    if "venv" in dir1:
        return s
    for i in os.listdir(dir1):
        i = str(i)
        p = os.path.join(dir1, i)
        if os.path.isdir(p):
            s += cool(p)
        else:
            if i.endswith(".cpp") or i.endswith(".h"):
                s += p + " "
    return s
os.chdir(os.path.join(os.getcwd(), "..\\"))
s = cool(".\\")
f = s.split(" ")
g = ""
g2 = ""
for p in f:
    if "firebase_cpp_sdk" in p:
        continue
    if len(p)>0:
        lf = count_lines(p)
        lines += lf
        files.append([p, lf])
    if (not p.endswith(".h")) and ("leaderboard" not in p.lower()) and ("firebase" not in p.lower()):
        g += (str(p).replace("\\","/")+ " ^\n")[2:]
    g2 += (str(p).replace("\\","/")+" ")[2:] + "\n"
print(g)
print(g2)

greatest_lines = -1
greatest_name = "none"

avg_lines = 0

for g in files:
    if g[1] > greatest_lines:
        greatest_lines = g[1]
        greatest_name = g[0]
    avg_lines += g[1]
    print(g[0]+" had " + str(g[1]) + " lines of code")

avg_lines /= len(files)

print("\nbiggest code file: " + greatest_name + ", with " + str(greatest_lines) + " lines of code")
print("avg lines per file: " + str(int(avg_lines)))
print("number of code files: " + str(len(files)))
print(str(f"{lines:,}") + " total lines of code")