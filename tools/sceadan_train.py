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

import sys,re,os,collections,random,multiprocessing
from subprocess import call,PIPE,Popen
from ttable import ttable

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
        
################################################################
## Sample Selection
################################################################

def filetypes():
    """Returns a list of the filetypes"""
    return [fn for fn in os.listdir(args.data) if os.path.isdir(os.path.join(args.data,fn))]

def ftype_files(ftype):
    """Returns a list of the pathnames for a give filetype in the training set"""
    ftypedir = os.path.join(args.data,ftype)
    return [os.path.join(ftypedir,fn) for fn in os.listdir(ftypedir)]

def train_files(ftype):
    """Returns all of the training files for a file type"""
    return db['train_files'][ftype]

def test_files(ftype):
    """Returns all of the test files for a file type"""
    return db['test_files'][ftype]



def blocks_in_file(fn):
    """ Return the number of blocks in a given file """
    return os.path.getsize(fn)//args.blocksize

def blocks_in_files(fns):
    return sum([blocks_in_file(f) for f in fns])

def split_data():
    """Takes data files and assigns them randomly to training
    and test data sets"""
    if 'test_files' in db:
        return
    the_test_files = {}
    the_train_files = {}
    for ftype in filetypes():
        files = ftype_files(ftype)
        random.shuffle(files)
        pivot = int(len(files) * args.split)
        the_test_files[ftype] = files[0:pivot]
        the_train_files[ftype] = files[pivot:]
    # Save in the shelf
    db['test_files'] = the_test_files
    db['train_files'] = the_train_files
        
    # Now find out how many blocks we want for each file type
    blocks = min([blocks_in_files(test_files[f]) for f in filetypes()])
    if args.maxblocks and blocks>args.maxblocks: blocks = arg.maxblocks
    db['blocks'] = blocks

    # For each file type, make a list of all the 
    for ftype in filetypes():
        blks = []
        for fn in test_files[ftype]:
            blks += [(fn,i) for i in range(0,blocks_in_file(fn))]
        random.shuffle(blks)
        db[ftype] = list(sorted(blks[0:blocks]))

def print_data():
    t = ttable()
    t.append_head(["FTYPE","Files","min blks","max blks","avg blks","total"])
    t.set_col_alignment(1,t.RIGHT)
    t.set_col_alignment(2,t.RIGHT)
    t.set_col_alignment(3,t.RIGHT)
    t.set_col_alignment(4,t.RIGHT)
    t.set_col_alignment(5,t.RIGHT)
    blocks_per_type = {}
    for ftype in filetypes():
        blocks = [os.path.getsize(fn)//args.blocksize for fn in ftype_files(ftype)]
        blocks = list(filter(lambda v:v>0,blocks))

        t.append_data((ftype,len(blocks),min(blocks),max(blocks),sum(blocks)/len(blocks),sum(blocks)))
        blocks_per_type[ftype] = sum(blocks)
    print(t.typeset(mode='text'))
    print("Sampling fraction: {}".format(args.split))
    print("Blocks per file type sample: {}".format(db['blocks']))
    if not args.verbose: return
    

    def print_files(ftype,ary):
        fmt = "{:40} {:8,}  {:8,}"
        total_blks = 0
        total_size = 0
        for fn in ary[ftype]:
            size = os.path.getsize(fn)
            blks = blocks_in_file(fn)
            print(fmt.format(fn,size,blks))
            total_size += size
            total_blks += blks
        print(fmt.format("     Total",total_size,total_blks))
    for ftype in filetypes():
        print("Filetype: {}".format(ftype))
        for what in ['test','train']:
            print(what+" data:")
            print_files(ftype,db[what+'_files'])
            print("\n")
        print("\n")
    
################################################################
## Training Vector Generation
################################################################

def generate_train_vectors_for_type(ftype):
    """Run sceadan for the specific blocks for each file. Output goes to a 
    temporary file whose name is returned to the caller."""

    outfn = os.path.join(args.exp,'vectors.'+ftype)
    out = open(outfn,"wb")
    cmd = [args.exe,'-b',str(args.blocksize),'-t',ftype,'-']
    p = Popen(cmd,stdout=out,stdin=PIPE)

    ret = ""
    blocks_by_file = {}
    for (fn,block) in db[ftype]:
        if fn not in blocks_by_file:
            blocks_by_file[fn] = set()
        blocks_by_file[fn].add(block)
    for (fn,blocks) in blocks_by_file.items():
        f = open(fn,"rb")
        for blk in blocks:
            f.seek(blk * args.blocksize)
            p.stdin.write(f.read(args.blocksize))
    p.stdin.close()
    p.wait()
    out.close()
    return outfn

def generate_train_vectors():
    train_file = os.path.join(args.exp,"vectors_train")
    tmp_file   = train_file+".tmp"
    if os.path.exists(train_file):
        print("Will not re-generate train vectors: {} already exists".format(train_file))
        return
    if os.path.exists(tmp_file): os.unlink(tmp_file)

    print("Generating train vectors with j={}".format(args.j))
    f = open(tmp_file,"wb")
    if args.j>1:
        import multiprocessing
        pool = multiprocessing.Pool(args.j)
    else:
        import multiprocessing.dummy
        pool = multiprocessing.dummy.Pool(1)
    for fn in pool.imap_unordered(generate_train_vectors_for_type,filetypes()):
        f.write(open(fn,"rb").read())
        os.unlink(fn)
    f.close()
    os.rename(tmp_file,train_file)


################################################################
### Generate a confusion matrix
################################################################
def generate_confusion():
    print("Generating confusion matrix")
    import re
    pat = re.compile("(\d+)\s+(.*) #")
    for ftype in filetypes():
        print("Filetype: ",ftype)
        for fn in test_files(ftype):
            print("  ",fn)
            cmd = [args.exe]
            if args.testblocksize:
                cmd += ['-b',str(args.testblocksize)]
            cmd += [fn]
            res = Popen(cmd,stdout=PIPE).communicate()[0].decode('utf-8')
            m = pat.search(res)
            print(m.group(2))
        print("\n")
        
            
################################################################
### Option Parsing
################################################################


help_text="""
Train sceadan from input files. File type
is determined by the containing directory name. If there is no containing directory
file type is determined by extension."""

if __name__=="__main__":
    import argparse,zipfile,os,time
    t0 = time.time()
    parser = argparse.ArgumentParser(description=help_text,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--data",help="Top directory of training/testing data",default='../DATA')
    parser.add_argument("--exp",help="Directory to hold experimental information",required=True)
    parser.add_argument("--split",help="Fraction of data to be used for testing",default=0.5)
    parser.add_argument("--maxblocks",type=int,help="Max blocks to use for training")
    parser.add_argument('--j',help='specify concurrency factor',type=int,default=1)
    
    parser.add_argument("--validate",help="Validate input data",action="store_true",default=True)
    parser.add_argument("--verbose",help="Print full detail",action='store_true')
    parser.add_argument('--blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--testblocksize',type=int,help='blocksize for testing.')
    parser.add_argument('--percentage',help='specifies percentage of blocks to sample',type=int,default=5)
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--extractlog',help='Recreate a training set with an extract log. The embedded filenames are relative to the location fo the extract.out log.',type=str)
    parser.add_argument('--maxsamples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--minfilesize',default=None,type=int)
    parser.add_argument('--notrain',action='store_true',help='Do not train a new model')
    parser.add_argument('--debug',action='store_true')

    args = parser.parse_args()

    if not os.path.exists(args.exp):
        os.mkdir(args.exp)

    t0 = time.time()

    import shelve
    db = shelve.open(os.path.join(args.exp,"experiment"))

    split_data()
    print_data()
    if not args.notrain:
        generate_train_vectors()
    generate_confusion()

    #outfile = open(args.outfile,'w')
    if args.extractlog:
        train_extractlog(args.extractlog)

    if args.files:
        if args.confusion:
            confusion_files()
        else:
            train_files()

    #outfile.close()
    print("Elapsed time: {:10.2g} seconds".format(time.time()-t0))
