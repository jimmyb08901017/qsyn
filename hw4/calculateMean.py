import sys
import os

def calculateMean(filename):
    mean = 0
    count = 0
    max = 0
    n_qubits = 0
    with open(filename, 'r') as f:
        line = f.readline()
        while line:
            if line.find("zxp") > 0:
                stats = f.readline()
                words = stats.split()
                index = -1
                for i in range(len(words)):
                    if words[i] == "outputs,":
                        index = i
                        break
                num = int(words[index+1])
                n_qubits = int(words[index-1])
                mean += num
                if num > max:
                    max = num
                count += 1
            line = f.readline()
    print("average:",mean/count - n_qubits*2)
    #print("max:",max - n_qubits*2)
    print()
    
    

if __name__ == "__main__":
    for file in sorted(os.listdir("./P3")):
        print(file)
        reading_file = "./P3/" + file
        calculateMean(reading_file)
