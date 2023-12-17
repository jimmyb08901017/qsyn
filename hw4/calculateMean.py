import sys

def calculateMean(filename):
    mean = 0
    count = 0
    max = 0
    with open(filename, 'r') as f:
        line = f.readline()
        while line:
            if line.find("zxp") > 0:
                stats = f.readline()
                words = stats.split()
                index = -1
                for i in range(len(words)):
                    if words[i] == "outputs,":
                        index = i+1
                        break
                num = int(words[index])
                mean += num
                if num > max:
                    max = num
                count += 1
            line = f.readline()
    print("average:",mean/count)
    print("max:",max)
    
    

if __name__ == "__main__":
    calculateMean(sys.argv[1])
