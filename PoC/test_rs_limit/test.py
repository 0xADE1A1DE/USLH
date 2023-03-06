#!/usr/bin/python3
import statistics
import csv
import sys
import subprocess as oss
import numpy as np
import matplotlib.pyplot as plt

def build():
	oss.call(['cc', 'main.c', '-O0'])

def run():
	oss.call(['bash', 'run.bash'])

def modify_line(file_name, line_num, new_instruction):
	lines = open(file_name, 'r').readlines()
	lines[line_num] = new_instruction
	out = open(file_name, 'w')
	out.writelines(lines)
	out.close();

def read_line(file_name):
	lines = open(file_name, 'r').readlines()
	return lines[0]


def main(repeat_time = 250, start_time=0):
	oss.call(['mkdir','data'])
	results = [0] * repeat_time
	threshold = 220
	for i in range(start_time,repeat_time+1):
		modify_line('./main.c', 29, 'asm volatile (\".rep ' + str(i) + '\");' + '\n')
		build()
		for j in range(1,101):
			run()
			results[i-1] = results[i-1] + int(read_line('tmp.txt'))
		print('', '%3d' % i, '', results[i-1] / 100)

	with open('data/slow_value2.txt', 'w') as f:
		for result in results:
			f.write(str(result/100))
			f.write('\n')

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        main(int(sys.argv[1]), int(sys.argv[2]))
    else:
        main()
