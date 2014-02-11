
import sys,os
from commands import getstatusoutput
Dir = sys.argv[1]	# a foler that contains 52 individual files
fail,ls = getstatusoutput('ls %s' % Dir)
if not fail:
	for filename in ls.split('\n'):
		if filename[-6:] in ['042332','082917','003644','075130']: # files that not used
			continue
		else:
			fp = open(Dir+'/'+filename,'r')
			for i,line in enumerate(fp):
				if i % 2 == 0:
					end = line.rfind('\"')
					print line[:end+1]
			fp.close()
else:
	print fail

