#!/usr/bin/python3.8

import sys
from os import system, listdir
from time import time
S = system

query_limit = 10000
if len(sys.argv) >= 3:
  query_limit = int(sys.argv[2])
limit = 60
if len(sys.argv) >= 4:
  limit = int(sys.argv[3])

S(f'cp ../archive/{sys.argv[1]} ../src/backtrack.cc')
S('make -C ../build')

query_list = sorted(listdir('../query'))
tot_cor = 0
tot_incor = 0
tot_time = 0
query_cnt = 0
logf = open('log_' + sys.argv[1].split('.')[0], 'w')
for query_name in query_list:
  query_cnt += 1
  if query_cnt > query_limit:
    break
  base_name = query_name.split('.')[0]
  qpath = '../query/' + query_name
  dpath = '../data/' + '_'.join(base_name.split('_')[:-1]) + '.igraph'
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

  logf.write(f'{base_name:>15} : {etime - stime:>6.3f} / {cor:>6} / {incor:>6}\n')

tot_cnt = len(query_list)
print('')
print('*********************************************')
print('            EXPERIMENT RESULT')
print('*********************************************')
print('')
print(f'Average Elapsed time  : {tot_time / tot_cnt:.3f} seconds')
print(f'Average Correct Ans   : {tot_cor / tot_cnt:.3f}')
print(f'Average Incorrect Ans : {tot_incor / tot_cnt:.3f}')

logf.write(f'TOT : time {tot_time / tot_cnt:.3f} / cor {tot_cor / tot_cnt:.3f} / incor {tot_incor / tot_cnt:.3f}\n')
logf.close()
