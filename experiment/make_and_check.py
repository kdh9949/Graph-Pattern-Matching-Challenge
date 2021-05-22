#!/usr/bin/python3.8

import sys
from os import system, listdir
from time import time
S = system

limit = 60
if len(sys.argv) == 3:
  limit = int(sys.argv[2])

S(f'cp ../archive/{sys.argv[1]} ../src/backtrack.cc')
S('make -C ../build')

query_list = sorted(listdir('../query'))
tot_cor = 0
tot_incor = 0
tot_time = 0
for query_name in query_list:
  base_name = query_name.split('.')[0]
  qpath = '../query/' + query_name
  dpath = '../data/' + base_name[:-3] + '.igraph'
  cpath = '../candidate_set/' + base_name + '.cs'
  opath = '../tmp/' + base_name + '.out'

  print('')
  print('---------------------------------')
  print('')
  print(f'Test for ({dpath}, {qpath})')
  
  stime = time()
  S(f'timeout {limit} ../build/main/program {dpath} {qpath} {cpath} > {opath}')
  etime = time()
  print(f'Elapsed time : {etime - stime:.3f} seconds')
  tot_time += etime - stime
  
  S(f'./score {dpath} {qpath} {opath} > ../tmp/score.out')
  g = open('../tmp/score.out', 'r')
  line = g.readline()
  print(line, end='')
  cor = int(line.split()[-1])
  line = g.readline()
  print(line, end='')
  incor = int(line.split()[-1])
  g.close()
  tot_cor += cor
  tot_incor += incor

tot_cnt = len(query_list)
print('')
print('*********************************************')
print('            EXPERIMENT RESULT')
print('*********************************************')
print('')
print(f'Average Elapsed time  : {tot_time / tot_cnt:.3f} seconds')
print(f'Average Correct Ans   : {tot_cor / tot_cnt:.3f}')
print(f'Average Incorrect Ans : {tot_incor / tot_cnt:.3f}')
