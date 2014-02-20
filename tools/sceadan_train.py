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

between = re.compile(".*/([^/]*)/")
def get_ftype(fn):
    ftype = os.path.splitext(fn.upper())[1][1:]
    if ftype.endswith("_INBOX"): ftype="TBIRD"
    if ftype.endswith("_TRASH"): ftype="TBIRD"
    if ftype.endswith("_SENT"): ftype="TBIRD"
    if ftype=="":
        m = between.search(fn)
        ftype=m.group(1)
    ftype = ftype_equivs.get(ftype,ftype)
    return ftype
    
def process_file(outfile,fn):
    ftype = get_ftype(fn)
    fin = open(fn,'rb')
    p1 = Popen([args.exe,'-b',str(args.blocksize),'-t',ftype,'-'],stdin=fin,stdout=PIPE)
    outfile.write(p1.communicate()[0].decode('utf-8'))

def ftype_from_name(fname):
    ftype = os.path.splitext(fname)[1][1:].lower()
    return ftype_equivs.get(ftype,ftype)

def process(fn_source,fname):
    """Process a file and return a list of the vectors"""

    if os.path.basename(fn_source)[0:1]=='.':
        print("Ignoring dot file",fn_source)
        return

    if args.minfilesize and os.path.getsize(fn_source) < args.minfilesize:
        print("Ignoring small file",fn_source)
        return

    from subprocess import call,PIPE,Popen
    ftype = ftype_from_name(fname)

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
    
def process_buf(outfile,fn,offset,bufsize):
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
    
def verify_extract(outfile,fn):
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
        process_buf(outfile,fn,int(offset)*BLOCKSIZE,BLOCKSIZE)
        count += 1
        if count%1000==0:
            print("Processed {:,} blocks".format(count))
        

if __name__=="__main__":
    import argparse,zipfile,os,time
    t0 = time.time()
    parser = argparse.ArgumentParser(description="Train sceadan from input files",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('files',action='store',help='input files for training. May be ZIP archives',nargs='?')
    parser.add_argument('--blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--outfile',help='output file for combined vectors',default='vectors.train')
    parser.add_argument('--outdir',help='output dir for vector segments',default='vectors.train')
    parser.add_argument('--percentage',help='specifies percentage of blocks to sample',type=int,default=5)
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--verify',help='Recreate a training set with an extract log. The embedded filenames are relative to the location fo the extract.out log.',type=str)
    parser.add_argument('--maxsamples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--minfilesize',default=None,type=int)
    parser.add_argument('--j',help='specify concurrency factor',type=int,default=1)
    parser.add_argument('--debug',action='store_true')
    parser.add_argument('--skip',type=int,help='Skip this many records (for testing)')
    args = parser.parse_args()

    t0 = time.time()
    outfile = open(args.outfile,'w')

    if args.verify:
        verify_extract(outfile,args.verify)

    # First create vectors from the inputs. Store the results in outfile
    def process_file(fn):
        if fn.lower().endswith('.zip'):
            z = zipfile.ZipFile(fn)
            for zfn in z.namelist():
                if zfn.endswith("/"): continue
                process(fn,zfn,iszipfile=True)
        else:
            process(fn,fn,iszipfile=False)

    if args.files:
        for fn in args.files:
            if os.path.isfile(fn):
                process_file(fn)
                continue
            if os.path.isdir(fn):
                for (dirpath,dirnames,filenames) in os.walk(fn):
                    for filename in filenames:
                        process_file(os.path.join(dirpath,filename))
        
    print("Counts of each block type:")
    for (ftype,count) in sorted(block_count.items()):
        print("{:10} {:10}".format(ftype,count))
    
    outfile.close()
    print("Elapsed time: {:10.2g} seconds".format(time.time()-t0))
