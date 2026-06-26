import shutil,os

for i in os.listdir(os.getcwd()):
    if "EXE Dev Build" in i:
        os.remove(i)

def clear(path):
    for p in os.listdir(path):
        f = path+"\\"+p
        if os.path.isfile(f):
            os.remove(f)
        else:
            clear(f)
    os.rmdir(path)
    

dst = ".EXE"

f = open("version", 'r')
version = int(f.readline())
f.close()
f = open("version",'w')
f.write(str(version+1))
f.close()

if not os.path.exists(dst):
    os.mkdir(dst)

p = '../cmake-build-release/BouncingPlus.exe'

if os.path.exists('../cmake-build-release/BouncingPlus.exe') and os.path.exists('../cmake-build-debug/BouncingPlus.exe') and os.path.exists('../cmake-build-optimized-release/BouncingPlus.exe'):
    f1 = os.path.getmtime('../cmake-build-release/BouncingPlus.exe')
    f2 = os.path.getmtime('../cmake-build-debug/BouncingPlus.exe')
    f3 = os.path.getmtime('../cmake-build-optimized-release/BouncingPlus.exe')

    print(f1, f2, f3)

    maxf = max(f1, max(f2, f3))
    if maxf == f1:
        print(maxf, f1)
        p = '../cmake-build-release/BouncingPlus.exe'
    elif maxf == f2:
        print(maxf, f2)
        p = '../cmake-build-debug/BouncingPlus.exe'
    elif maxf == f3:
        print(maxf, f3)
        p = '../cmake-build-optimized-release/BouncingPlus.exe'

p = '../cmake-build-optimized-release/BouncingPlus.exe'
print(p)
shutil.copy(p, dst)
for file in os.listdir(p.replace("/BouncingPlus.exe", "")):
    if file.endswith(".dll"):
        shutil.copy(p.replace("BouncingPlus.exe", "")+file, dst)
shutil.copytree("../assets", dst+"/assets")

epik_file_name = "Bouncing Plus - Version "+str(version)+" (EXE Dev Build)"

shutil.make_archive(epik_file_name, 'zip', dst)

clear(str(os.path.abspath(dst)))
