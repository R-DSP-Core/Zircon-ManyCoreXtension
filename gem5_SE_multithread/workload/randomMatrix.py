import numpy as np 

I = 16
K = 32
J = 16


A = np.random.normal(-1., 5., (I, K))
B = np.random.normal(1., 7., (J, K))

#file = [open(r"C:\Users\Lenovo\Desktop\A0.txt", "w+"), 
#             open(r"C:\Users\Lenovo\Desktop\A1.txt", "w+"),
#             open(r"C:\Users\Lenovo\Desktop\A2.txt", "w+"),
#             open(r"C:\Users\Lenovo\Desktop\A3.txt", "w+")]
file1 = open(r"C:\Users\Lenovo\Desktop\B.txt", "w+")
file2 = open(r"C:\Users\Lenovo\Desktop\A.txt", "w+")
file3 = open(r"C:\Users\Lenovo\Desktop\C.txt", "w+")

#for f in file:
#    f.write("{")

file1.write("{")
file2.write("{")

#for i in range(0, int(I/4)):
#    for j in range(0, int(K)):
#        for k in range(0, 4):
 #           file[k].write(f"{A[i+k*I/4, j]}, ")

for i in range(0,int(J)):
    for j in range(0, int(K)): 
        file1.write(f"{B[i, j]}, ")
        
for i in range(0, int(I)):
    for j in range(0, int(K)):
        file2.write(f"{A[i, j]}, ")
        
#for f in file:
 #   f.write("};")
file1.write("};")
file2.write("};")

C = np.matmul(A, np.transpose(B))

for i in range(0, int(I)):
    for j in range(0, int(J)):
        file3.write(f"{C[i, j]}, ")

print()