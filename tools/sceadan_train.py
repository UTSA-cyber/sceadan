#!/usr/bin/env python3
#
# revised training and regression test for sceadan
#

ftype_equivs = {"jpeg":"jpg",
                "dll":"exe"}


import collections

block_count = collections.defaultdict(int) # number of blocks of each file type
file_count  = collections.defaultdict(int) # number of files of each file type

def ftype_from_name(fname):
    ftype = os.path.splitext(fname)[1][1:].lower()
    return ftype_equivs.get(ftype,ftype)

def process(fn_source,fname,iszipfile=False):
    """Process a file and return a list of the vectors"""

    if os.path.basename(fn_source)[0:1]=='.':
        print("Ignoring dot file",fn_source)
        return

    if os.path.getsize(fn_source) < args.minfilesize:
        print("Ignoring small file",fn_source)
        return

    from subprocess import call,PIPE,Popen
    ftype = ftype_from_name(fname)

    if iszipfile and ftype=='':
        ftype = os.path.splitext(os.path.basename(fn_source))[0]

    # See if we need more of this type
    if block_count[ftype] > args.samples: return

    if iszipfile:
        p0    = Popen(['unzip','-p',fn_source,fname],stdout=PIPE)
        fin   = p0.stdout
        fname = '-'             # read from stdin

    else:
        fin   = None

    # Collect the data points

    # At this point we have a stream(fin) and a type. See if we need to collect more

    cmd = [args.exe,'-b',str(args.blocksize),'-t',ftype,'-p',str(args.percentage),'-P',fname]
    p1 = Popen(cmd,stdin=fin,stdout=PIPE,stderr=PIPE)
    res          = p1.communicate()
    if not res[0] and not res[1]:
        return                  # no data, no problem (might be a sampling issue)
    vectors      = res[0].decode('utf-8').split('\n')
    offsets      = res[1].decode('utf-8').split('\n')
    assert(len(vectors)==len(offsets))

    # Finally, add to the file
    with open(args.outdir+"/"+ftype,"a") as f:
        for (v,o) in zip(vectors,offsets):
            try:
                f.write("{}  # {}\n".format(v,o))
                block_count[ftype] += 1
            except OSError as e:
                print("{} f.write error: {}".format(fname,str(e)))

        file_count[ftype] += 1
        print("{} {} {}".format(fname,ftype,block_count[ftype]))
    
def run_grid():
    from distutils.spawn import find_executable
    train = find_executable('train')
    
if __name__=="__main__":
    import argparse,zipfile,os
    parser = argparse.ArgumentParser(description="Train sceadan from input files",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('files',action='store',help='input files for training. May be ZIP archives',nargs='+')
    parser.add_argument('--blocksize',type=int,default=512,help='blocksize for training.')
    parser.add_argument('--outfile',help='output file for combined vectors',default='vectors.train')
    parser.add_argument('--outdir',help='output dir for vector segments',default='vectors.train')
    parser.add_argument('--percentage',help='specifies percentage of blocks to sample',type=int,default=5)
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    parser.add_argument('--minfilesize',default=4096*2,type=int)
    parser.add_argument('--j',help='specify concurrency factor',type=int,default=1)
    args = parser.parse_args()

    # First create vectors from the inputs. Store the results in outfile

    outfile = open(args.outfile,'w')

    def process_file(fn):
        if fn.lower().endswith('.zip'):
            z = zipfile.ZipFile(fn)
            for zfn in z.namelist():
                if zfn.endswith("/"): continue
                process(fn,zfn,iszipfile=True)
        else:
            process(fn,fn,iszipfile=False)

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
