#!/usr/bin/python3.8

from os import system, listdir
from time import time
S = system

S('cd ../build')
S('make')
S('cd ../experiment')

query_list = sorted(listdir('../query'))
for query_name in query_list:
  qpath = '../query/' + query_name
  dpath = '../data/' + query_name.split('.')[0][:-3] + '.igraph'
  cpath = '../candidate_set/' + query_name.split('.')[0] + '.cs'
  
  print('')
  print('---------------------------------')
  print('')
  print(f'Test for ({dpath}, {qpath})')
  stime = time()
  S(f'timeout 60 ../build/main/program {dpath} {qpath} {cpath} > ../tmp/out.txt')
  etime = time()
  print(f'Elapsed time : {etime - stime:.3f} seconds')
  S(f'./score {dpath} {qpath} ../tmp/out.txt')
