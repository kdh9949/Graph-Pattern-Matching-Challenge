#!/usr/bin/python3.8

import sys
from os import system, listdir
from time import time
S = system

if len(sys.argv) != 4:
  print('Usage : ./stress_test [code filename] [trial for each test] [time limit for each test]')

code_name = sys.argv[1]
trial = int(sys.argv[2])
limit = int(sys.argv[3])
logf = open('log_' + code_name.split('.')[0], 'w')

S(f'cp ../archive/{sys.argv[1]} ../src/backtrack.cc')
S('make -C ../build')

data_list = sorted(listdir('../data'))
for data_name in data_list:
  print('------------------------------')
  print(f'Now testing {data_name}')
  print('------------------------------\n')
  logf.write(f'--- {data_name} ---\n')

  dpath = '../data/' + data_name
  qpath = '../tmp/query'
  cpath = '../tmp/cs'
  opath = '../tmp/out'
  spath = '../tmp/score'
  
  for k in range(1, 5):
    n = k * (10 if 'human' in data_name else 50)
    print(f'n = {n}')
    
    for s in range(3):
      min_deg = 2.0 + 0.3 * s
      max_deg = min_deg + 0.3
      print(f'sparsity : {min_deg:.1f} ~ {max_deg:.1f}\n')

      avg_time = 0
      solved_cnt = 0
      correct = True
      
      for i in range(trial):
        print(f'Trial {i + 1}:')
        S(f'./gen_query {dpath} {qpath} {cpath} {n} {min_deg:.1f} {max_deg:.1f}')
      
        stime = time()
        S(f'timeout {limit} ../build/main/program {dpath} {qpath} {cpath} > {opath}')
        elapsed_time = time() - stime
        avg_time += min(limit, elapsed_time)
        print(f'Elapsed {elapsed_time:.3f} seconds')

        S(f'./score {dpath} {qpath} {opath} > {spath}')
        g = open(spath, 'r')
        cor = int(g.readline().split()[-1])
        incor = int(g.readline().split()[-1])
        g.close()
        print(f'Cor : {cor} / Incor : {incor}\n')
        if incor > 0:
          correct = False
          break
        if elapsed_time < limit - 0.1 and cor > 0:
          solved_cnt += 1
      
      avg_time /= trial
  
      print(f'Finished {data_name} / n = {n} / sparsity {min_deg:.1f} ~ {max_deg:.1f}\n')
      if not correct:
        logf.write(f'{n:>3} / {min_deg:.1f}  :  Wrong Program')
      else:
        logf.write(f'{n:>3} / {min_deg:.1f}  :  {avg_time:>6.3f}s / {100 * solved_cnt / trial:.1f}%')
        logf.write(f' ({solved_cnt} / {trial})\n')
