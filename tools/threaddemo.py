#!/usr/bin/env python


def f(x):
    print("x=",x)
    return "okay" + x

if __name__=="__main__":
    names = ["line %d" % i for i in range(1,100)]
    from multiprocessing.pool import Pool
    with Pool(processes=4) as pool:
        result = pool.map(f,names)
    print("final result:")
    print(result)
