# http://docs.python.org/3.3/library/multiprocessing.html
# http://stackoverflow.com/questions/14592531/threading-in-python-doesnt-happen-parallel/14594205

from multiprocessing.dummy import Pool

def generate_data(): # generate some dummy urls
    for i in range(1000):
        print("yield",i)
        yield i

def process(data):
    return data,data*data

pool = Pool(20) # limit number of concurrent connections
for data, result in pool.imap_unordered(process,generate_data()):
    print("data=",data,"result=",result)
