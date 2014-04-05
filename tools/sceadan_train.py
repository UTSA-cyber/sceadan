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
if sys.version_info < (3,3):
    raise RuntimeError("Requires Python 3.3 or above")


import sys,re,os,collections,random,multiprocessing
from subprocess import call,PIPE,Popen
from ttable import ttable
from collections import defaultdict

block_count = collections.defaultdict(int) # number of blocks of each file type
file_count  = collections.defaultdict(int) # number of files of each file type
OpenMP_j    = 4                            # since we compiled with OpenMP, -j4 is enough


################################################################
### Utilitiy functions
################################################################

def expname(fn):
    return os.path.join(args.exp,fn)

def openexp(fn,mode):
    return open(expname(fn),mode)

def train_file():
    return os.path.join(args.exp,"vectors_to_train")

def model_file():
    return os.path.join(args.exp,"model")

def hms_time(label,t):
    t = int(t * 100)/100
    return "{}: {} ({} seconds)".format(label,str(datetime.timedelta(seconds=t)),t)

def sceadan_type_for_name(name):
    return int(Popen([args.exe,'-T',name],stdout=PIPE).communicate()[0])

def sceadan_name_for_type(t):
    return Popen([args.exe,'-T',str(t)],stdout=PIPE).communicate()[0]


################################################################
## Sample Selection
## This creates the blocks used for training and testing.
################################################################

def datadir():
    return args.data

def filetypes(upper=False):
    """Returns a list of the filetypes"""
    ftypes = [fn for fn in os.listdir(args.data) if os.path.isdir(os.path.join(args.data,fn))]
    ftypes.sort()
    if upper:
        ftypes = [fn.upper() for fn in ftypes]
    return ftypes
    
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
    return os.path.getsize(fn)//args.train_blocksize

def blocks_in_files(fns):
    return sum([blocks_in_file(f) for f in fns])

def split_data():
    """Takes data files and assigns them randomly to training
    and test data sets...

    - Randomly partition the files into 1/2 training, 1/2 testing
    - Compute the number of blocks present for each file type's train and test parts. 
    - Take the minimum of these numbers. 
    - Randomly chose that number of blocks from each file type of the train set.
    """
    must_split = False
    for key in ['test_files','train_files','blocks']:
        print(key,key in db.keys())
        if key not in db.keys():
            must_split = True
    if not must_split:
        return
    print("Splitting data...")
    the_test_files = {}
    the_train_files = {}
    for ftype in filetypes():
        files = ftype_files(ftype)
        random.shuffle(files)
        pivot = int(len(files) * args.split)
        the_test_files[ftype] = files[0:pivot]
        the_train_files[ftype] = files[pivot:]

    # Save in the shelve
    db['test_files'] = the_test_files
    db['train_files'] = the_train_files
        
    # Now find out how many blocks we want for each file type
    blocks = min([blocks_in_files(the_test_files[f]) for f in filetypes()])
    if args.maxblocks and blocks>args.maxblocks: blocks = arg.maxblocks
    db['blocks'] = blocks
    db['train_blocksize'] = args.train_blocksize

    # For each file type, make a list of all the blocks
    for ftype in filetypes():
        blks = []
        for fn in the_test_files[ftype]:
            blks += [(fn,i) for i in range(0,blocks_in_file(fn))]
        random.shuffle(blks)
        db[ftype] = list(sorted(blks[0:blocks]))

def print_data():
    print("Data directory:",datadir())
    t = ttable()
    t.append_head(["FTYPE","Files","min blks","max blks","avg blks","total"])
    t.set_col_alignment(1,t.RIGHT)
    t.set_col_alignment(2,t.RIGHT)
    t.set_col_alignment(3,t.RIGHT)
    t.set_col_alignment(4,t.RIGHT)
    t.set_col_alignment(5,t.RIGHT)
    blocks_per_type = {}
    for ftype in filetypes():
        if sceadan_type_for_name(ftype) == -1:
            raise RuntimeError("file type {} is invalid".format(ftype))

        blocks = [os.path.getsize(fn)//args.train_blocksize for fn in ftype_files(ftype)]
        blocks = list(filter(lambda v:v>0,blocks))
        t.append_data((ftype,len(blocks),min(blocks),max(blocks),sum(blocks)/len(blocks),sum(blocks)))
        blocks_per_type[ftype] = sum(blocks)
    print(t.typeset(mode='text'))

def print_sample():
    print("Sampling fraction: {}".format(args.split))
    print("Blocks per file type in sample:",db['blocks'])
    print("Total vectors in train set:",db['blocks']*len(db.keys()))
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
    
def validate_train_file():
    if not train_file():
        print("No train file to validate")
        return
    print("Train file:",train_file())
    linecount = 0
    for line in open(train_file(),"r"):
        items = line.strip().split(" ")
        indexes = [int(v.split(":")[0]) for v in items[1:]]
        values  = [float(v.split(":")[0]) for v in items[1:]]
        assert(indexes==sorted(indexes))
        linecount += 1
    print("Total validated lines:",linecount)

################################################################
## Training Vector Generation
################################################################

def generate_train_vectors_for_type(ftype):
    """Run sceadan for the specific blocks for a specific file
    type. Output goes to a temporary file whose name is returned to
    the caller. The files are generated in parallel with the
    multiprocessing module (this is a slow process), the combined and
    unlinked by the main thread (that is a fast process).
    """

    outfn = os.path.join(args.exp,'vectors.'+ftype)
    out = open(outfn,"wb")
    cmd = [args.exe,'-b',str(args.train_blocksize),'-t',ftype,'-']
    if args.ngram_mode:
        cmd += ['-n',str(args.ngram_mode)]
    db['train_blocksize'] = args.train_blocksize
    db['vector_generation_cmd'] = " ".join(cmd)
    p = Popen(cmd,stdout=out,stdin=PIPE)
    ret = ""
    blocks_by_file = {}
    for (fn,block) in db[ftype]:
        if args.train_noblock0 and block==0:
            continue            # do not add block 0?
        if fn not in blocks_by_file: 
            blocks_by_file[fn] = set()
        blocks_by_file[fn].add(block)
    for (fn,blocks) in blocks_by_file.items():
        f = open(fn,"rb")
        for blk in blocks:
            f.seek(blk * args.train_blocksize)
            p.stdin.write(f.read(args.train_blocksize))
    p.stdin.close()
    p.wait()
    out.close()
    return outfn

def generate_train_vectors():
    """Generate the training vectors for liblinear from the train database."""
    if os.path.exists(train_file()):
        print("Will not re-generate train vectors: {} already exists".format(train_file()))
        return
    tmp_file   = train_file()+".tmp"
    if os.path.exists(tmp_file): os.unlink(tmp_file)

    print("Generating train vectors with j={}".format(args.j))
    t1 = time.time()
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
    os.rename(tmp_file,train_file())
    print(hms_time("Time to generate training vectors",time.time()-t0))

def train_model():
    import sys

    #
    # First run grid.py
    #
    if os.path.exists(model_file()):
        print("Will not regenerate {}: already exists".format(model_file()))
        return

    dataset_out = os.path.join(args.exp,'dataset.out')
    if not args.nogrid:
        cmd  = [sys.executable,'grid.py']
        cmd += ['-j',str(OpenMP_j)]             # since we compile with OpenMP, -4 is enough
        cmd += ['-log2g','null','-gnuplot','null']
        cmd += ['-svmtrain',args.trainexe]
        cmd += ['-out',dataset_out]
        cmd += [train_file()]
        print(" ".join(cmd))
        t0 = time.time()
        call(cmd)
        t = time.time()-t0
        print(hms_time("Grid Search time",t))
        db['grid_cmd'] = cmd
        db['grid_time'] = t
    #
    # Now run the trainer
    #
    c = args.c
    if not c:
        # Try to read from the file
        best_rate = 0
        pat = re.compile("log2c=(\d+) rate=([\d\.]+)")
        for line in open(dataset_out,'r'):
            m = pat.search(line)
            if m:
                this_c = 2**int(m.group(1))
                this_rate = float(m.group(2))
                if this_rate > best_rate:
                    best_rate = this_rate
                    c = this_c
        print("Using c={} (best rate={}) from file".format(c,best_rate))
    cmd = [args.trainexe,'-e',"{}".format(args.epsilon),'-c',str(c),train_file(),model_file()]
    t0 = time.time()
    call(cmd)
    db['liblinear_train_command'] = " ".join(cmd)
    print(hms_time("Time to train",time.time()-t0))


################################################################
### Generate a confusion matrix
################################################################
def get_sceadan_score_for_file(fn,tally):
    """Score a file, optionally with test blocksize."""

def get_sceadan_score_for_filetype(ftype):
    from collections import defaultdict

    print("Generating sceadan score for file type {}".format(ftype))
    tally = defaultdict(int)
    import re
    pat = re.compile("(\d+)\s+(.*) #")
    cmd = [args.exe]
    
    # Compute the file count and the block count
    file_count = len(test_files(ftype))
    block_count = blocks_in_files(test_files())
    if args.test_noblock0:
        block_count -= file_count # ignoring the first block of each file

    if args.test_blocksize:
        cmd += ['-b',str(args.test_blocksize)]
    if not args.nomodel:
        cmd += ['-m',model_file()]
    if args.ngram_mode:
        cmd += ['-n',args.ngram_mode]
    if args.test_noblock0:
        cmd += ['-x']
    cmd += test_files(ftype)
    sceadan_classification_cmd = " ".join(cmd)
    if args.verbose:
        print("cmd:",cmd[0:120])
    res = Popen(cmd,stdout=PIPE).communicate()[0].decode('utf-8')
    for line in res.split("\n"):
        if line=="": continue
        m = pat.search(line)
        if m:
            tally[m.group(2)] += 1
        else:
            print("Weird line:",line)
    print (ftype,"classifications: ",tally)
    return (ftype,tally,file_count,block_count)


def generate_confusion():
    if os.path.exists(expname("confusion.txt")):
        print("Confusion matrix already exists")
        return

    if not os.path.exists(model_file()):
        raise RuntimeError("Cannot generate confusion matrix. Model file {} does not exist".format(model_file()))
        
    print("Generating confusion matrix")
    t = ttable()

    db['test_blocksize'] = args.test_blocksize

    #t.append_head(['File Type','Classifies as'])

    # Okay. Get these in a row with a threadpool
    if args.j>1:
        import multiprocessing
        pool = multiprocessing.Pool(args.j)
    else:
        import multiprocessing.dummy
        pool = multiprocessing.dummy.Pool(1)
    sceadan_score_rows = {}
    files_per_type     = {}
    blocks_per_type    = {}
    classified_types = set()
    for (ftype,tally,file_count,block_count) in pool.imap_unordered(get_sceadan_score_for_filetype,filetypes()):
        sceadan_score_rows[ftype] = tally
        classified_types = classified_types.union(set(tally.keys()))
        files_per_type[ftype] = file_count
        blocks_per_type[ftype] = block_count

    classtypes = [t.upper() for t in sorted(classified_types)]

    # Get a list of all the classified types
    # And calculate the precision and recall

    t.append_head(['    ','file ', 'block'])
    t.append_head(['type','count', 'count'] + classtypes)
    total_events = 0
    total_correct = 0
    percent_correct_sum= 0
    rowcounter = 0
    for ftype in filetypes():
        FTYPE         = ftype.upper()
        tally         = sceadan_score_rows[ftype]
        count         = sum(tally.values())
        total_events  += count
        total_correct += tally.get(ftype.upper(),0)
        percent_correct = tally.get(ftype.upper(),0)*100.0 / count
        percent_correct_sum += percent_correct

        # Generate each row of the output
        
        data = [FTYPE, files_per_type[FTYPE], blocks_per_type[FTYPE]] + ["{:3.0f}".format(tally.get(f,0)*100.0/count) for f in classtypes]
        t.append_data(data)
        rowcounter += 1
        if rowcounter % 5 ==0:
            t.append_data([''])
        f = openexp("confusion.txt","w")
        f.write(t.typeset(mode='text'))
        print(t.typeset(mode='text'))
    db['overall_accuracy'] = (total_correct*100.0)/total_events
    db['average_accuracy_per_class'] = percent_correct_sum/len(filetypes())
        
    def info(n):
        """Print a value from the database and print in confusion.txt"""
        try:
            v = str(db[n])[0:120]
            val = "{}: {}".format(n.replace("_"," "),v)
            f.write(val+"\n")
            print(val)
        except KeyError as e:
            f.write("key {} not found\n".format(n))
    print("Keys in database:",list(db.keys()))
    info('train_blocksize')
    info('test_blocksize')
    info('vector_generation_cmd')
    info('liblinear_train_command')
    info('overall_accuracy')
    info('average_accuracy_per_class')
        
            
################################################################
### Option Parsing
################################################################


def stest():
    db = shelve.open("stest",writeback=True)
    db['foo'] = "bar"
    db['baz'] = [1,2,3]
    db.close()
    db = shelve.open("stest",writeback=True)
    for (k,v) in db.items():
        print(k,"=",v)
    exit(0)

help_text="""
Train sceadan from input files. File type
is determined by the containing directory name. If there is no containing directory
file type is determined by extension."""

if __name__=="__main__":
    import argparse,zipfile,os,time,shelve,datetime,shutil
    t0 = time.time()
    print("="*60)
    print("="*60)
    print("="*60)
    print(" ".join(sys.argv))
    print(" ")
    parser = argparse.ArgumentParser(description=help_text,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--data",help="Top directory of training/testing data",default='../DATA')
    parser.add_argument("--exp",help="Directory to hold experimental information")
    parser.add_argument("--copyexp",help="Copy experimental data from this directory")
    parser.add_argument("--copymodel",help="Copy model from this directory")
    parser.add_argument("--split",help="Fraction of data to be used for testing",default=0.5)
    parser.add_argument("--maxblocks",type=int,help="Max blocks to use for training")
    parser.add_argument('--j',help='specify concurrency factor',type=int,default=1)
    parser.add_argument("--trainexe",help="Liblinear train executable",default='../liblinear-1.94-omp/train')
    parser.add_argument("--verbose",help="Print full detail",action='store_true')
    parser.add_argument('--train_blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--test_blocksize',type=int,help='blocksize for testing.')
    parser.add_argument('--train_noblock0',help='Do not train on block 0',action='store_true')
    parser.add_argument('--test_noblock0',help='Do not test on block 0',action='store_true')
    parser.add_argument('--exe',help='Specify name of sceadan_app',default=os.path.join(os.path.dirname(__file__),'../src/sceadan_app'))
    parser.add_argument('--note',help='Add a note to the expeirment archive')
    parser.add_argument('--notrain',action='store_true',help='Do not train a new model')
    parser.add_argument('--c',help='Specify C parameter for training')
    parser.add_argument('--epsilon',help='Error for training',type=float,default=0.01)
    parser.add_argument('--nogrid',help='Do not use a grid search to find c',action='store_true')
    parser.add_argument('--nomodel',help='Use built-in model',action='store_true')
    parser.add_argument('--ngram_mode',help='ngram mode',type=str)
    parser.add_argument('--validate',help='Just validate the test data',action='store_true')
    parser.add_argument("--dbdump",help="Dump the named database")
    parser.add_argument("--stest",help="test the shelf",action='store_true')
    
    #parser.add_argument('--percentage',help='specifies percentage of blocks to sample',type=int,default=5)
    #parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    #parser.add_argument('--maxsamples',help='Number of samples needed for each type',default=10000,type=int)
    #parser.add_argument('--minfilesize',default=None,type=int)

    args = parser.parse_args()

    # Check to make sure files exists
    if not os.path.exists(args.exe):
        raise RuntimeError("exe {} not found".format(args.exe))
    if not os.path.exists(args.trainexe):
        raise RuntimeError("trainexe {} not found".format(args.trainexe))

    if args.stest: stest()      # shelf test

    if args.validate:
        print_data()
        if args.exp: validate_train_file()
        exit(0)

    if args.dbdump:
        db = shelve.open(args.dbdump,writeback=True)
        for (key,val) in db.items():
            print("{}={}",(key,val))
        exit(0)

    if not args.exp:
        print("--exp <DIR> must be provided")
        exit(1)
    if not os.path.exists(args.exp):
        os.mkdir(args.exp)

    if args.copyexp:
        print("Copying training data from {} to {}".format(args.copyexp,args.exp))
        for fn in ['experiment']:
            shutil.copyfile(os.path.join(args.copyexp,fn),os.path.join(args.exp,fn))

    if args.copymodel:
        print("Copying model from {} to {}".format(args.copyexp,args.exp))
        for fn in ['experiment','model']:
            shutil.copyfile(os.path.join(args.copymodel,fn),os.path.join(args.exp,fn))

    print("Starting run at {}".format(time.asctime()))

    if args.note:
        with openexp("note.txt","a") as f:
            f.write(time.asctime()+"\n")
            f.write(args.note+"\n")
            

    t0 = time.time()

    db = shelve.open(expname("experiment"),writeback=True)
    split_data()
    print_data()

    if not args.notrain:
        generate_train_vectors()
        train_model()
    generate_confusion()

    sec = time.time() - t0
    print(hms_time("Elapsed time",sec))
    db.close()
