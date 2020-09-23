import os

f = open(os.path.expanduser("~/Desktop/test_file_in.txt"), "w")
[f.write("r") for i in range(10**8)]; 
f.close()
