#!/usr/bin/env python
# This gridregression.py is modified from grid.py in LIBSVM 3.17 version
__all__ = ['find_parameters']

import os, sys, traceback, getpass, time, re
from threading import Thread
from subprocess import *

if sys.version_info[0] < 3:
	from Queue import Queue
else:
	from queue import Queue

telnet_workers = []
ssh_workers = []
nr_local_worker = 1

class GridOption:
	def __init__(self, dataset_pathname, options):
		dirname = os.path.dirname(__file__)
		if sys.platform != 'win32':
			self.svmtrain_pathname = os.path.join(dirname, '../svm-train')
			self.gnuplot_pathname = '/usr/bin/gnuplot'
		else:
			# example for windows
			self.svmtrain_pathname = os.path.join(dirname, r'..\windows\svm-train.exe')
			# svmtrain_pathname = r'c:\Program Files\libsvm\windows\svm-train.exe'
			self.gnuplot_pathname = r'c:\tmp\gnuplot\binary\pgnuplot.exe'
		self.fold = 5
		self.c_begin, self.c_end, self.c_step = -1,  6,  1
		self.g_begin, self.g_end, self.g_step =  0, -8, -1
		self.p_begin, self.p_end, self.p_step = -8,  -1,  1
		self.grid_with_c, self.grid_with_g, self.grid_with_p = True, True, True
		self.dataset_pathname = dataset_pathname
		self.dataset_title = os.path.split(dataset_pathname)[1]
		self.out_pathname = '{0}.out'.format(self.dataset_title)
		self.png_pathname = '{0}.png'.format(self.dataset_title)
		self.pass_through_string = ' '
		self.resume_pathname = None
		self.parse_options(options)

	def parse_options(self, options):
		if type(options) == str:
			options = options.split()
		i = 0
		pass_through_options = []
		
		while i < len(options):
			if options[i] == '-log2c':
				i = i + 1
				if options[i] == 'null':
					self.grid_with_c = False
				else:
					self.c_begin, self.c_end, self.c_step = map(float,options[i].split(','))
			elif options[i] == '-log2g':
				i = i + 1
				if options[i] == 'null':
					self.grid_with_g = False
				else:
					self.g_begin, self.g_end, self.g_step = map(float,options[i].split(','))
			elif options[i] == '-log2p' :
				i = i + 1
				if options[i] == 'null':
					self.grid_with_p = False	
				else:
					self.p_begin, self.p_end, self.p_step = map(float,options[i].split(','))
			elif options[i] == '-v':
				i = i + 1
				self.fold = options[i]
			elif options[i] in ('-c','-g','-p'):
				raise ValueError('Use -log2c , -log2g and -log2p.')
			elif options[i] == '-svmtrain':
				i = i + 1
				self.svmtrain_pathname = options[i]
			elif options[i] == '-gnuplot':
				i = i + 1
				if options[i] == 'null':
					self.gnuplot_pathname = None
				else:	
					self.gnuplot_pathname = options[i]
			elif options[i] == '-out':
				i = i + 1
				if options[i] == 'null':
					self.out_pathname = None
				else:
					self.out_pathname = options[i]
			elif options[i] == '-png':
				i = i + 1
				self.png_pathname = options[i]
			elif options[i] == '-resume':
				if i == (len(options)-1) or options[i+1].startswith('-'):
					self.resume_pathname = self.dataset_title + '.out'
				else:
					i = i + 1
					self.resume_pathname = options[i]
			else:
				pass_through_options.append(options[i])
			i = i + 1

		self.pass_through_string = ' '.join(pass_through_options)
		if not os.path.exists(self.svmtrain_pathname):
			raise IOError('svm-train executable not found')
		if not os.path.exists(self.dataset_pathname):
			raise IOError('dataset not found')
		if self.resume_pathname and not os.path.exists(self.resume_pathname):
			raise IOError('file for resumption not found')
		if not self.grid_with_c and not self.grid_with_g and not self.grid_with_p:
			raise ValueError('-log2c , -log2g and -log2p should not be null simultaneously')
		# gridregression default did not support gnuplot
		self.gnuplot_pathname = None
		if self.gnuplot_pathname and not os.path.exists(self.gnuplot_pathname):
			sys.stderr.write('gnuplot executable not found\n')
			self.gnuplot_pathname = None

def redraw(db,best_param,gnuplot,options,tofile=False):
	if len(db) == 0: return
	begin_level = round(max(x[2] for x in db)) - 3
	step_size = 0.5

	best_log2c,best_log2g,best_rate = best_param

	# if newly obtained c, g, or cv values are the same,
	# then stop redrawing the contour.
	if all(x[0] == db[0][0]  for x in db): return
	if all(x[1] == db[0][1]  for x in db): return
	if all(x[2] == db[0][2]  for x in db): return

	if tofile:
		gnuplot.write(b"set term png transparent small linewidth 2 medium enhanced\n")
		gnuplot.write("set output \"{0}\"\n".format(options.png_pathname.replace('\\','\\\\')).encode())
		#gnuplot.write(b"set term postscript color solid\n")
		#gnuplot.write("set output \"{0}.ps\"\n".format(options.dataset_title).encode().encode())
	elif sys.platform == 'win32':
		gnuplot.write(b"set term windows\n")
	else:
		gnuplot.write( b"set term x11\n")
	gnuplot.write(b"set xlabel \"log2(C)\"\n")
	gnuplot.write(b"set ylabel \"log2(gamma)\"\n")
	gnuplot.write("set xrange [{0}:{1}]\n".format(options.c_begin,options.c_end).encode())
	gnuplot.write("set yrange [{0}:{1}]\n".format(options.g_begin,options.g_end).encode())
	gnuplot.write(b"set contour\n")
	gnuplot.write("set cntrparam levels incremental {0},{1},100\n".format(begin_level,step_size).encode())
	gnuplot.write(b"unset surface\n")
	gnuplot.write(b"unset ztics\n")
	gnuplot.write(b"set view 0,0\n")
	gnuplot.write("set title \"{0}\"\n".format(options.dataset_title).encode())
	gnuplot.write(b"unset label\n")
	gnuplot.write("set label \"Best log2(C) = {0}  log2(gamma) = {1}  accuracy = {2}%\" \
				  at screen 0.5,0.85 center\n". \
				  format(best_log2c, best_log2g, best_rate).encode())
	gnuplot.write("set label \"C = {0}  gamma = {1}\""
				  " at screen 0.5,0.8 center\n".format(2**best_log2c, 2**best_log2g).encode())
	gnuplot.write(b"set key at screen 0.9,0.9\n")
	gnuplot.write(b"splot \"-\" with lines\n")
	
	db.sort(key = lambda x:(x[0], -x[1]))

	prevc = db[0][0]
	for line in db:
		if prevc != line[0]:
			gnuplot.write(b"\n")
			prevc = line[0]
		gnuplot.write("{0[0]} {0[1]} {0[2]}\n".format(line).encode())
	gnuplot.write(b"e\n")
	gnuplot.write(b"\n") # force gnuplot back to prompt when term set failure
	gnuplot.flush()


def calculate_jobs(options):
	
	def range_f(begin,end,step):
		# like range, but works on non-integer too
		seq = []
		while True:
			if step > 0 and begin > end: break
			if step < 0 and begin < end: break
			seq.append(begin)
			begin = begin + step
		return seq
	
	def permute_sequence(seq):
		n = len(seq)
		if n <= 1: return seq
	
		mid = int(n/2)
		left = permute_sequence(seq[:mid])
		right = permute_sequence(seq[mid+1:])
	
		ret = [seq[mid]]
		while left or right:
			if left: ret.append(left.pop(0))
			if right: ret.append(right.pop(0))
			
		return ret	

	
	c_seq = permute_sequence(range_f(options.c_begin,options.c_end,options.c_step))
	g_seq = permute_sequence(range_f(options.g_begin,options.g_end,options.g_step))
	p_seq = permute_sequence(range_f(options.p_begin,options.p_end,options.p_step))
	
	if not options.grid_with_c:
		c_seq = [None]
	if not options.grid_with_g:
		g_seq = [None] 
	if not options.grid_with_p:
		p_seq = [None]
	
	nr_c = len(c_seq)
	nr_g = len(g_seq)
	nr_p = len(p_seq)
	jobs = []

	for i in range(0,nr_c):
		for j in range(0,nr_g):
			for s in range(0,nr_p):
				line=[]
				line.append((c_seq[i],g_seq[j],p_seq[s]))
				jobs.append(line)
	resumed_jobs = {}
	
	if options.resume_pathname is None:
		return jobs, resumed_jobs

	for line in open(options.resume_pathname, 'r'):
		line = line.strip()
		rst = re.findall(r'mse=([0-9.]+)',line)
		if not rst: 
			continue
		mse = float(rst[0])

		c, g, p = None, None, None 
		rst = re.findall(r'log2c=([0-9.-]+)',line)
		if rst: 
			c = float(rst[0])
		rst = re.findall(r'log2g=([0-9.-]+)',line)
		if rst: 
			g = float(rst[0])
		rst = re.findall(r'log2p=([0-9,-]+)',line)
		if rst:
			p = float(rst[0])

		resumed_jobs[(c,g,p)] = mse

	return jobs, resumed_jobs

	
class WorkerStopToken:  # used to notify the worker to stop or if a worker is dead
	pass

class Worker(Thread):
	def __init__(self,name,job_queue,result_queue,options):
		Thread.__init__(self)
		self.name = name
		self.job_queue = job_queue
		self.result_queue = result_queue
		self.options = options
		
	def run(self):
		while True:
			(cexp,gexp,pexp) = self.job_queue.get()
			if cexp is WorkerStopToken:
				self.job_queue.put((cexp,gexp,pexp))
				# print('worker {0} stop.'.format(self.name))
				break
			try:
				c, g, p = None, None, None
				if cexp != None:
					c = 2.0**cexp
				if gexp != None:
					g = 2.0**gexp
				if pexp != None:
					p = 2.0**pexp
				mse = self.run_one(c,g,p)
				if mse is None: raise RuntimeError('get no mse')
			except:
				# we failed, let others do that and we just quit
			
				traceback.print_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2])
				
				self.job_queue.put((cexp,gexp,pexp))
				sys.stderr.write('worker {0} quit.\n'.format(self.name))
				break
			else:
				self.result_queue.put((self.name,cexp,gexp,pexp,mse))

	def get_cmd(self,c,g,p):
		options=self.options
		cmdline = options.svmtrain_pathname
		cmdline += ' -s 3 '
		if options.grid_with_c: 
			cmdline += ' -c {0} '.format(c)
		if options.grid_with_g: 
			cmdline += ' -g {0} '.format(g)
		if options.grid_with_p:
			cmdline += ' -p {0}' .format(p)
		cmdline += ' -v {0} {1} {2} '.format\
			(options.fold,options.pass_through_string,options.dataset_pathname)
		return cmdline
		
class LocalWorker(Worker):
	def run_one(self,c,g,p):
		cmdline = self.get_cmd(c,g,p)
		result = Popen(cmdline,shell=True,stdout=PIPE,stderr=PIPE,stdin=PIPE).stdout
		for line in result.readlines():
			if str(line).find('Cross') != -1:
				return float(line.split()[-1])

class SSHWorker(Worker):
	def __init__(self,name,job_queue,result_queue,host,options):
		Worker.__init__(self,name,job_queue,result_queue,options)
		self.host = host
		self.cwd = os.getcwd()
	def run_one(self,c,g,p):
		cmdline = 'ssh -x -t -t {0} "cd {1}; {2}"'.format\
			(self.host,self.cwd,self.get_cmd(c,g,p))
		result = Popen(cmdline,shell=True,stdout=PIPE,stderr=PIPE,stdin=PIPE).stdout
		for line in result.readlines():
			if str(line).find('Cross') != -1:
				return float(line.split()[-1])

class TelnetWorker(Worker):
	def __init__(self,name,job_queue,result_queue,host,username,password,options):
		Worker.__init__(self,name,job_queue,result_queue,options)
		self.host = host
		self.username = username
		self.password = password		
	def run(self):
		import telnetlib
		self.tn = tn = telnetlib.Telnet(self.host)
		tn.read_until('login: ')
		tn.write(self.username + '\n')
		tn.read_until('Password: ')
		tn.write(self.password + '\n')

		# XXX: how to know whether login is successful?
		tn.read_until(self.username)
		# 
		print('login ok', self.host)
		tn.write('cd '+os.getcwd()+'\n')
		Worker.run(self)
		tn.write('exit\n')			   
	def run_one(self,c,g,p):
		cmdline = self.get_cmd(c,g,p)
		result = self.tn.write(cmdline+'\n')
		(idx,matchm,output) = self.tn.expect(['Cross.*\n'])
		for line in output.split('\n'):
			if str(line).find('Cross') != -1:
				return float(line.split()[-1])
			
def find_parameters(dataset_pathname, options=''):
	
	def update_param(c,g,p,mse,best_c,best_g,best_p,best_mse,worker,resumed):
		if ( mse < best_mse ):
			best_mse,best_c,best_g,best_p = mse,c,g,p
		stdout_str = '[{0}] {1} {2} (best '.format\
			(worker,' '.join(str(x) for x in [c,g,p] if x is not None),mse)
		output_str = ''
		if c != None:
			stdout_str += 'c={0}, '.format(2.0**best_c)
			output_str += 'log2c={0} '.format(c)
		if g != None:
			stdout_str += 'g={0}, '.format(2.0**best_g)
			output_str += 'log2g={0} '.format(g)
		if p != None:
			stdout_str += 'p={0}, '.format(2.0**best_p)
			output_str += 'log2p={0} '.format(p)
		stdout_str += 'mse={0})'.format(best_mse)
		print(stdout_str)
		if options.out_pathname and not resumed:
			output_str += 'mse={0}\n'.format(mse)
			result_file.write(output_str)
			result_file.flush()
		
		return best_c,best_g,best_p,best_mse
		
	options = GridOption(dataset_pathname, options);

	if options.gnuplot_pathname:
		gnuplot = Popen(options.gnuplot_pathname,stdin = PIPE,stdout=PIPE,stderr=PIPE).stdin
	else:
		gnuplot = None
		
	# put jobs in queue

	jobs,resumed_jobs = calculate_jobs(options)
	job_queue = Queue(0)
	result_queue = Queue(0)

	for (c,g,p) in resumed_jobs:
		result_queue.put(('resumed',c,g,p,resumed_jobs[(c,g,p)]))

	for line in jobs:
		for (c,g,p) in line:
			if (c,g,p) not in resumed_jobs:
				job_queue.put((c,g,p))

	# hack the queue to become a stack --
	# this is important when some thread
	# failed and re-put a job. It we still
	# use FIFO, the job will be put
	# into the end of the queue, and the graph
	# will only be updated in the end
 
	job_queue._put = job_queue.queue.appendleft

	# fire telnet workers

	if telnet_workers:
		nr_telnet_worker = len(telnet_workers)
		username = getpass.getuser()
		password = getpass.getpass()
		for host in telnet_workers:
			worker = TelnetWorker(host,job_queue,result_queue,
					 host,username,password,options)
			worker.start()

	# fire ssh workers

	if ssh_workers:
		for host in ssh_workers:
			worker = SSHWorker(host,job_queue,result_queue,host,options)
			worker.start()

	# fire local workers

	for i in range(nr_local_worker):
		worker = LocalWorker('local',job_queue,result_queue,options)
		worker.start()

	# gather results

	done_jobs = {}

	if options.out_pathname:
		if options.resume_pathname:
			result_file = open(options.out_pathname, 'a')
		else:
			result_file = open(options.out_pathname, 'w')


	db = []
	best_mse = float('+inf')
	best_c,best_g,best_p = None,None,None  

	for (c,g,p) in resumed_jobs:
		mse = resumed_jobs[(c,g,p)]
		best_c,best_g,best_p,best_mse = update_param(c,g,p,mse,best_c,best_g,best_p,best_mse,'resumed',True)

	for line in jobs:
		for (c,g,p) in line:
			while (c,g,p) not in done_jobs:
				(worker,c1,g1,p1,mse1) = result_queue.get()
				done_jobs[(c1,g1,p1)] = mse1
				if (c1,g1,p1) not in resumed_jobs:
					best_c,best_g,best_p,best_mse = update_param(c1,g1,p1,mse1,best_c,best_g,best_p,best_mse,worker,False)
			db.append((c,g,p,done_jobs[(c,g,p)]))
		if gnuplot and options.grid_with_c and options.grid_with_g:
			redraw(db,[best_c, best_g, best_rate],gnuplot,options)
			redraw(db,[best_c, best_g, best_rate],gnuplot,options,True)


	if options.out_pathname:
		result_file.close()
	job_queue.put((WorkerStopToken,None,None))
	best_param, best_cgp  = {}, []
	if best_c != None:
		best_param['c'] = 2.0**best_c
		best_cgp += [2.0**best_c]
	if best_g != None:
		best_param['g'] = 2.0**best_g
		best_cgp += [2.0**best_g]
	if best_p != None:
		best_param['p'] = 2.0**best_p
		best_cgp += [2.0**best_p]
	print('{0} {1}'.format(' '.join(map(str,best_cgp)), best_mse))

	return best_mse, best_param


if __name__ == '__main__':

	def exit_with_help():
		print("""\
Usage: gridregression.py [grid_options] [svm_options] dataset

grid_options :
-log2c {begin,end,step | "null"} : set the range of c (default -1,6,1)
    begin,end,step -- c_range = 2^{begin,...,begin+k*step,...,end}
    "null"         -- do not grid with c
-log2g {begin,end,step | "null"} : set the range of g (default 0,-8,-1)
    begin,end,step -- g_range = 2^{begin,...,begin+k*step,...,end}
    "null"         -- do not grid with g
-log2p {begin,end,step | "null"} : set the range of p (default -8, -1, 1)
    begin,end,step -- p_range = 2^{begin,...,begin+k*step,...,end}
    "null"         -- do not grid with p
-v n : n-fold cross validation (default 5)
-svmtrain pathname : set svm executable path and name
-out {pathname | "null"} : (default dataset.out)
    pathname -- set output file path and name
    "null"   -- do not output file
-resume [pathname] : resume the grid task using an existing output file (default pathname is dataset.out)
    This is experimental. Try this option only if some parameters have been checked for the SAME data.

svm_options : additional options for svm-train""")
		sys.exit(1)
	
	if len(sys.argv) < 2:
		exit_with_help()
	dataset_pathname = sys.argv[-1]
	options = sys.argv[1:-1]
	try:
		find_parameters(dataset_pathname, options)
	except (IOError,ValueError) as e:
		sys.stderr.write(str(e) + '\n')
		sys.stderr.write('Try "gridregression.py" for more information.\n')
		sys.exit(1)
