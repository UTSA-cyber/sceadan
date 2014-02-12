#!/usr/bin/env python3
#
# revised training and regression test for sceadan
#

def process(outfile,fn,fname,iszipfile=False):
    from subprocess import call,PIPE,Popen
    ftype = os.path.splitext(fname.lower())[1][1:]

    if iszipfile:
        p0 = Popen(['unzip','-p',fn,fname],stdout=PIPE)
        fin = p0.stdout
    else:
        fin = open(fn,'rb')

    p1 = Popen([args.exe,'-b',str(args.blocksize),'-t',ftype,'-'],stdin=fin,stdout=PIPE)
    outfile.write(p1.communicate()[0].decode('utf-8'))

    
def run_grid():
    from distutils.spawn import find_executable
    train = find_executable('train')
    
    

if __name__=="__main__":
    import argparse,zipfile,os
    parser = argparse.ArgumentParser(description="Train sceadan from input files",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('files',action='store',help='input files for training. May be ZIP archives',nargs='+')
    parser.add_argument('--blocksize',type=int,default=4096,help='blocksize for training.')
    parser.add_argument('--outfile',help='output file for combined vectors',default='vectors.train')
    parser.add_argument('--exe',help='Specify name of sceadan_app',default='../src/sceadan_app')
    parser.add_argument('--samples',help='Number of samples needed for each type',default=10000,type=int)
    args = parser.parse_args()

    # First create vectors from the inputs. Store the results in outfile

    outfile = open(args.outfile,'w')
    for fn in args.files:
        if fn.lower().endswith('.zip'):
            z = zipfile.ZipFile(fn)
            for zfn in z.namelist():
                process(outfile,fn,zfn,iszipfile=True)
        else:
            process(outfile,fn,fn,iszipfile=False)
    
    outfile.close()
