import os
import subprocess
import string

f = open("stats.logs", "r")

lines = f.readlines()

satisfied = {}
throughput = {}
vnfs = {}
nodes = {}
tat = {}
time = {}
count = {}

for i in range(len(lines)):
	line = lines[i].split()
	if(i%6==0):
		algo = line[2]
		if(algo not in satisfied):
			satisfied[algo]={}
		requests = float(line[4])
		sat = float(line[3])
		if(requests not in satisfied[algo]):
			satisfied[algo][requests] = sat
		else:
			satisfied[algo][requests] += sat 

		if(algo not in count):
			count[algo]={}
		if(requests not in count[algo]):
			count[algo][requests] = 1
		else:
			count[algo][requests] += 1 	
	
	elif(i%6==1):
		thru = float(line[2])
		if(algo not in throughput):
			throughput[algo]={}
		if(requests not in throughput[algo]):
			throughput[algo][requests] = thru
		else:
			throughput[algo][requests] += thru 	

	elif(i%6==2):
		vnf = float(line[6])
		if(algo not in vnfs):
			vnfs[algo]={}
		if(requests not in vnfs[algo]):
			vnfs[algo][requests] = vnf
		else:
			vnfs[algo][requests] += vnf 
	
	elif(i%6==3):
		node = float(line[4])
		if(algo not in nodes):
			nodes[algo]={}
		if(requests not in nodes[algo]):
			nodes[algo][requests] = node
		else:
			nodes[algo][requests] += node 
	
	elif(i%6==4):
		t = float(line[4])
		if(algo not in tat):
			tat[algo]={}
		if(requests not in tat[algo]):
			tat[algo][requests] = t
		else:
			tat[algo][requests] += t 
	
	elif(i%6==5):
		times = line[5]
		if(algo not in time):
			time[algo]={}
		if(requests not in time[algo]):
			time[algo][requests] = times
		else:
			time[algo][requests] += times 


f.close()

for algo in satisfied:
	for requests in satisfied[algo]:
		satisfied[algo][requests] = satisfied[algo][requests]/count[algo][requests]
		throughput[algo][requests] = throughput[algo][requests]/count[algo][requests] 
		vnfs[algo][requests] = vnfs[algo][requests]/count[algo][requests] 
		nodes[algo][requests] = nodes[algo][requests]/count[algo][requests] 
		tat[algo][requests] = tat[algo][requests]/count[algo][requests] 
		time[algo][requests] = time[algo][requests]/count[algo][requests] 

f = open("results.log", "w")

for type1 in sorted(dict.keys()):
	f.write("type = "+str(type1)+"\n")
	for threads in sorted(dict[type1].keys()):
		f.write(str(threads) + "\t" + str(dict[type1][threads]) + "\n")

f.close()