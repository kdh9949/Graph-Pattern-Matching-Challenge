#!/usr/bin/python3.8

from os import system, listdir
from time import time
S = system

S('make -C ../build')

query_list = sorted(listdir('../query'))
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
  S(f'timeout 60 ../build/main/program {dpath} {qpath} {cpath} > {opath}')
  etime = time()
  print(f'Elapsed time : {etime - stime:.3f} seconds')
  S(f'./score {dpath} {qpath} {opath}')
