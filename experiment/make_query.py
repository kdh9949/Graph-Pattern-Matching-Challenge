#!/usr/bin/python3.8

import sys
from os import system, listdir
S = system

if len(sys.argv) != 8:
  print('Usage : ./make_query [data graph name] [k for 1k, 2k, ...] [query prefix] [min avg deg] [max avg deg] [n for 1k, 2k, ... nk] [# of data for each param]')
  exit()

data_name = sys.argv[1]
k = int(sys.argv[2])
prefix = sys.argv[3]
min_avgdeg = sys.argv[4]
max_avgdeg = sys.argv[5]
A = int(sys.argv[6])
B = int(sys.argv[7])

base_name = data_name.split('.')[0]
for i in range(A):
  for j in range(B):
    query_name = base_name + '_m' + prefix + str(i + 1) + str(j + 1) + '.igraph'
    command = f'./gen_query {data_name} {query_name} {(i + 1) * k} {min_avgdeg} {max_avgdeg}'
    print(command)
    S(command)
