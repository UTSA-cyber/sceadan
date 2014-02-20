#!/usr/bin/env python3
#
# Simson L. Garfinkel, Naval Postgraduate School
# Public domain
#
# revised training and regression test for sceadan
#
#
# 2014-02-19 - removed automated handling of ZIP files; it slowed things down and seeking was hard.
#            - Provided support for reading an extract.out file         

import sys
from subprocess import call,PIPE,Popen
import re

between = re.compile("/(.*)/")
def get_ftype(fn):
    ftype = os.path.splitext(fn.upper())[1][1:]
    if ftype=="":
        m = between.search(fn)
        ftype=m.group(1)
    return ftype
    
        

def process_file(outfile,fn):
    ftype = get_ftype(fn)
    fin = open(fn,'rb')
    p1 = Popen([args.exe,'-b',str(args.blocksize),'-t',ftype,'-'],stdin=fin,stdout=PIPE)
    outfile.write(p1.communicate()[0].decode('utf-8'))

    
def process_buf(outfile,fn,offset,bufsize):
    ftype = get_ftype(fn)
    with open(fn,"rb") as fin:
        fin.seek(offset)
        buf = fin.read(bufsize)
        p1 = Popen([args.exe,'-b',str(bufsize),'-t',ftype,'-'],stdin=fin,stdout=PIPE)
        outfile.write(p1.communicate()[0].decode('utf-8'))

def run_grid():
    from distutils.spawn import find_executable
    train = find_executable('train')
    
    
def verify_extract(outfile,fn):
    dname = os.path.dirname(fn)
    alerted = set()
    for line in open(fn):
        (path,offset) = line.strip().split('\t')
        fn = os.path.join(dname,path)
        if not os.path.exists(fn):
            if fn not in alerted:
                alerted.add(fn)
                print("Does not exist: {}".format(fn),file=sys.stderr)
            continue
        BLOCKSIZE = 512
        process_buf(outfile,fn,int(offset)*BLOCKSIZE,BLOCKSIZE)
        

if __name__=="__main__":
    import argparse,zipfile,os,time
    parser = argparse.ArgumentParser(description="Train sceadan from input files",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('files',action='store',help='input files for training. May be ZIP archives',nargs='?')
    parser.add_argument('--blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--outfile',help='output file for combined vectors',default='vectors.train')
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--verify',help='Recreate a training set with an extract log. The embedded filenames are relative to the location fo the extract.out log.',type=str)
    args = parser.parse_args()

    t0 = time.time()
    outfile = open(args.outfile,'w')

    if args.verify:
        verify_extract(outfile,args.verify)


    # First create vectors from the inputs. Store the results in outfile

    for fn in args.files:
        process_file(outfile,fn)
    outfile.close()
    print("elapsed time: {}".format(time.time()-t0))
