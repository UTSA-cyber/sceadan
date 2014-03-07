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
import collections

block_count = collections.defaultdict(int) # number of blocks of each file type
file_count  = collections.defaultdict(int) # number of files of each file type


ftype_equivs = {"JPEG":"JPG",
                "DLL":"EXE",
                "TXT":"TEXT",
                "TIFF":"TIF",
                "MSF":"TBIRD",
                "URLENCODED":"URL"}

def get_ftype(fn):
    ftype = os.path.basename(os.path.dirname(fn))
    if ftype: return ftype.upper()
    ftype = os.path.splitext(fn.upper())[1][1:]
    return ftype_equivs.get(ftype,ftype)
    
def train_file(fname):
    """Process a file and send a list of vectors to output file"""

    if args.minfilesize and os.path.getsize(fname) < args.minfilesize:
        print("Ignoring small file",fname)
        return
    
    ftype = get_ftype(fname)

    # See if we need more of this type
    if block_count[ftype] > args.maxsamples: return

    # Collect the data points

    cmd = [args.exe,'-b',str(args.blocksize),'-t',ftype,'-p',str(args.percentage),'-P',fname]
    p1 = Popen(cmd,stdout=PIPE,stderr=PIPE)
    res          = p1.communicate()
    if not res[0] and not res[1]:
        return                  # no data, no problem (might be a sampling issue)
    vectors      = res[0].decode('utf-8').split('\n')
    offsets      = res[1].decode('utf-8').split('\n')
    if len(vectors)!=len(offsets):
        print(offsets)
        return

    assert(len(vectors)==len(offsets))
    
    # Finally, add to the file
    with open(args.outdir+"/"+ftype,"a") as f:
        for (v,o) in zip(vectors,offsets):
            if len(v)<5: continue # not possibly valid
            try:
                f.write("{}  # {}\n".format(v,o))
                block_count[ftype] += 1
            except OSError as e:
                print("{} f.write error: {}".format(fname,str(e)))

        file_count[ftype] += 1
        print("{} {} {}".format(fname,ftype,block_count[ftype]))
    

confusion={}
def confusion_file(fname):
    """Process a file and report how each block is classified"""
    ftype = get_ftype(fname)
    cmd = [args.exe,'-b',str(args.blocksize),fname]
    p1  = Popen(cmd,stdout=PIPE,stderr=PIPE)
    res = p1.communicate()
    r   = re.compile("(\d+)\s+([^ ]+)")
    for line in res[0].decode('utf-8').split('\n'):
        m = r.search(line)
        if not m: continue
        btype = m.group(2)
        if ftype not in confusion: confusion[ftype] = {}
        if btype not in confusion[ftype]: confusion[ftype][btype] = 0
        confusion[ftype][btype] += 1
    print(confusion)


def process_fname_dir(files,func):
    print("pf = ",files)
        
def train_files():
    for fname in args.files:
        if os.path.isfile(fname): train_file(fname)
        if os.path.isdir(fname):
            for (dirpath,dirnames,filenames) in os.walk(fname):
                for filename in filenames:
                    if filename[0:1]=='.': continue # ignore dot files
                    train_file(os.path.join(dirpath,filename))
    print("Counts of each block type:")
    for (ftype,count) in sorted(block_count.items()):
        print("{:10} {:10}".format(ftype,count))
    
def confusion_files():
    for fname in args.files:
        if os.path.isfile(fname): confusion_file(fname)
        if os.path.isdir(fname):
            for (dirpath,dirnames,filenames) in os.walk(fname):
                for filename in filenames:
                    if filename[0:1]=='.': continue # ignore dot files
                    confusion_file(os.path.join(dirpath,filename))


def train_buf(outfile,fn,offset,bufsize):
    if args.skip:
        args.skip -= 1
        return
    ftype = get_ftype(fn)
    with open(fn,"rb") as fin:
        cmd = [args.exe,'-b',str(bufsize),'-t',ftype,'-']
        if args.debug:
            print(fn)
            print(" ".join(cmd),file=sys.stderr)
        p1 = Popen(cmd,stdin=PIPE,stdout=PIPE)
        fin.seek(offset)
        p1.stdin.write(fin.read(bufsize))
        fin.close()
        res = p1.communicate()
        vectors = res[0].decode('utf-8')
        outfile.write(vectors)
        block_count[ftype] += 1

def run_grid():
    from distutils.spawn import find_executable
    train = find_executable('train')
    
def train_extractlog(fn):
    dname = os.path.dirname(fn)
    alerted = set()
    count = 0
    for line in open(fn):
        (path,offset) = line.strip().split('\t')
        fn = os.path.join(dname,path)
        if not os.path.exists(fn):
            if fn not in alerted:
                alerted.add(fn)
                print("Does not exist: {}".format(fn),file=sys.stderr)
                continue
        BLOCKSIZE = 512
        train_buf(outfile,fn,int(offset)*BLOCKSIZE,BLOCKSIZE)
        count += 1
        if count%1000==0:
            print("Processed {:,} blocks".format(count))
        
help_text="""
Train sceadan from input files. File type
is determined by the containing directory name. If there is no containing directory
file type is determined by extension."""

if __name__=="__main__":
    import argparse,zipfile,os,time
    t0 = time.time()
    parser = argparse.ArgumentParser(description=help_text,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('files',action='store',help="input files or directories for training. ",nargs='+')
    parser.add_argument('--blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--outfile',help='output file for combined vectors',default='vectors.train')
    parser.add_argument('--outdir',help='output dir for vector segments',default='vectors.train')
    parser.add_argument('--percentage',help='specifies percentage of blocks to sample',type=int,default=5)
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--extractlog',help='Recreate a training set with an extract log. The embedded filenames are relative to the location fo the extract.out log.',type=str)
    parser.add_argument('--maxsamples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--minfilesize',default=None,type=int)
    parser.add_argument('--j',help='specify concurrency factor',type=int,default=1)
    parser.add_argument('--skip',type=int,help='Skip this many records (for testing)')
    parser.add_argument('--confusion',action='store_true',help='Generate a confusion matrix')
    parser.add_argument('--debug',action='store_true')

    args = parser.parse_args()

    t0 = time.time()
    outfile = open(args.outfile,'w')

    if args.extractlog:
        train_extractlog(args.extractlog)

    if args.files:
        if args.confusion:
            confusion_files()
        else:
            train_files()

    outfile.close()
    print("Elapsed time: {:10.2g} seconds".format(time.time()-t0))
