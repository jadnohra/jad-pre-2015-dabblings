import socket
import sys
import os
import traceback
import errno
import time
import threading
import platform
import subprocess
import shutil
from sets import Set
from collections import namedtuple
import mediafrost as frost

gpio = None
has_rpi = False
def rpiBegin(pins):
	global gpio
	global has_rpi
	global rpi_status
	if (has_rpi):
		return
	has_rpi = False
	try:
		import RPi.GPIO as gpio
		gpio.setmode(gpio.BOARD)
		gpio.setwarnings(False)
		print 'Detected Raspberri GPIO. pins: ',
		for p in pins:
			gpio.setup(p, gpio.OUT)
			print '{}={},'.format(p, gpio.input(p)),
		print ''
		has_rpi = True
	except RuntimeError:
		has_gpio = False
		str = traceback.format_exc()
		if ('root' in str):
			print str
		has_gpio = False
	return has_rpi

def rpiEnd():
	global has_rpi
	if (not has_rpi):
		return
	gpio.cleanup()
	has_rpi = False

def rpiSetGpio(pin, high):
	global has_rpi
	if (not has_rpi):
		return
	gpio.output(pin, high)	

def rpiGetGpio(pin):
    global has_rpi
    if (not has_rpi):
        return
    return (gpio.input(pin) == gpio.HIGH)

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_lcache = os.path.join(self_dir, 'lcache')
self_mount = os.path.join(self_dir, 'mount')
self_test_out = os.path.join(self_dir, 'test_out')

warn = ('-warn' in sys.argv)
dry = ('-dry' in sys.argv)
scan = ('-scan' in sys.argv)
arg_db = None
if ('-db' in sys.argv):
	arg_db = sys.argv[sys.argv.index('-db')+1]
db_add = ('-db_add' in sys.argv)	
no_am = ('-no_am' in sys.argv)
no_ap = ('-no_ap' in sys.argv)
perfile = ('-perfile' in sys.argv)
dbg = ('-dbg' in sys.argv)
dbg2 =('-dbg2' in sys.argv)
dbg3 = ('-dbg3' in sys.argv)
dbgsvn = ('-dbgsvn' in sys.argv)
local_cache = ('-local_cache' in sys.argv)
lim_cache = 0
if ('-lim_cache' in sys.argv):
	lim_cache = int(sys.argv[sys.argv.index('-lim_cache')+1])
no_rpi = ('-no_rpi' in sys.argv)
pup_delay = 5; pdown_delay = 1;
if ('-pup_delay' in sys.argv):
    pup_delay = int(sys.argv[sys.argv.index('-pup_delay')+1])
if ('-pdown_delay' in sys.argv):
    pdown_delay = int(sys.argv[sys.argv.index('-pdown_delay')+1])
if (not no_rpi):
	rpiBegin([12])
	if (not has_rpi):
		print 'Failed to connected to RaspberryPi.'
		exit(0)
	

def resetCache(path, limit, minimum=1024*1024*256, unused=1024*1024*32):
	if (os.path.isdir(path)):
		map( os.unlink, [os.path.join(path,f) for f in os.listdir(path) if os.path.isfile(f)] )
	else:
		os.makedirs(path)
	statvfs = os.statvfs(path)
	cache_size = statvfs.f_frsize*statvfs.f_bavail
	cache_size = cache_size-unused
	if (limit):
		if cache_size > limit:
			cache_size = limit
		else:
			cache_size = 0
	else:
		if (cache_size <= minimum):
			cache_size = 0
	return cache_size

def niceBytes(bytes):
	a = ['bytes', 'KB', 'MB', 'GB']
	r = float(bytes); i = 0; 
	while (i<len(a) and  (r/1024.0 > 1.0)):	
		r = r / 1024.0; i=i+1;
	return '{:.2f} {}'.format(r, a[i])

default_mount_match_name = '???'
sys_plat = platform.system().lower()
if ('darwin' in sys_plat):
	default_mount_match_name = 'Apple'
if ('linux' in sys_plat):
	default_mount_match_name = '/dev/root'

if ('-test_out' in sys.argv) or ('-ctest_out' in sys.argv):
	if (os.path.isdir(self_test_out) and ('-ctest_out' in sys.argv)):
		shutil.rmtree(self_test_out)
	if (not os.path.isdir(self_test_out)):
		os.makedirs(self_test_out)
	
	fs_target_filters = [ frost.FsMountPointFilter(True, 'test_fs_out', 'test_fs_out','test_fs_out', default_mount_match_name, '0', self_test_out) ]
else:
	fs_target_filters = []

fs_mounts = frost.fsFindMounts()
if dbg3:
    print 'fs_mounts:', fs_mounts

fs_lcache_filters = [ frost.FsMountPointFilter(True, 'fs_lcache', 'fs_lcache','fs_lcache', default_mount_match_name, '0', self_lcache) ]
fs_mcache_filters = [ frost.FsMountPointFilter(True, 'fs_mcache', 'fs_mcache','fs_mcache', default_mount_match_name, '0', os.path.join(self_mount,'mcache/mediafrost/mcache')) ]
fs_cache_sources = None
fs_lcache_sources = None

if (local_cache):
	fs_lcache_sources = frost.fsFilterMounts(fs_mounts, fs_lcache_filters, True, True)
	lcache_size = resetCache(self_lcache, lim_cache)
	print '{} of local cache available.'.format(niceBytes(lcache_size))
	if (lcache_size <= 0):
		exit(0)
else:
	print 'Will use mounted cache.'

fs_am_filters = [
				frost.FsMountPointFilter(True, 'fs_back1', 'Vault_Jad','Vault_Jad', default_mount_match_name, '1000', os.path.join(self_mount,'vault_jad/Vault/mediafrost')),
				frost.FsMountPointFilter(True, 'fs_back2', 'Vault_Lena','Vault_Lena', default_mount_match_name, '1000', os.path.join(self_mount,'vault_lena/Vault/mediafrost')),
             ]
FsAutoMount = namedtuple('FsAutoMount', 'filter uuid local_path')
fs_ams = [
            FsAutoMount(fs_am_filters[0], '150B-2565', 'vault_jad'),
            FsAutoMount(fs_am_filters[1], '1101-1163', 'vault_lena'),
            FsAutoMount(fs_mcache_filters[0], '60CD-BC6D', 'mcache'),
        ]
if (no_am):
    fs_am_filters = []; fs_ams = [];
fs_am_status = {}
fs_am_targets = []


fs_manual_targets = frost.fsFilterMounts(fs_mounts, fs_target_filters, warn, True)
if dbg3:
	print 'fs_target_filters:', fs_target_filters
	print 'fs_manual_targets:', fs_manual_targets
use_ui = ('-ui' in sys.argv)
if use_ui:
    frost.bkpUiChooseStoragePoints([], fs_manual_targets)


def fsAmScan(silent=False):
	#lsusb
	if (not silent):
		print 'Scanning auto-mounts...'	 
	if (not os.path.isdir(self_mount)):
		os.makedirs(self_mount)
	cands = {}
	out = subprocess.Popen(['ls', '-laF', '/dev/disk/by-uuid/'], stdout=subprocess.PIPE).stdout.read()
	for line in iter(out.splitlines()):
		data = line.split()
		if ((len(data)>3) and (data[-2] == '->') and ('/sd' in data[-1])):
			cands[data[-3]] = '/dev/' + data[-1].split('/')[-1]
	return cands	

def fsAmPowerUp(earlyOutCallback=None, callbackArgs=None):
	if (no_ap):
		return
	if ((not has_rpi) or rpiGetGpio(12)):
		return
	print 'Powering up...',
	rpiSetGpio(12, True)
	for i in range(pup_delay):
		if (earlyOutCallback and earlyOutCallback(callbackArgs)):
			print '(callback hit)',
			break
		print pup_delay-i,'',
		sys.stdout.flush()
		time.sleep(1)
	print ''	

def fsAmPowerDown():
	if (no_ap):
		return
	if ((not has_rpi) or (not rpiGetGpio(12))):
		return
	print 'Powering down...',
	for i in range(pdown_delay):
		print pdown_delay-i,'',
		sys.stdout.flush()
		time.sleep(1)
	print ''
	rpiSetGpio(12, False)

def fsAmUnmount(am, dev):
	mpath = os.path.join(self_mount, am.local_path)
	out = subprocess.Popen(['mountpoint', mpath], stdout=subprocess.PIPE).stdout.read()
	is_mounted = ('is a mountpoint' in out)
	if (not is_mounted):
		return True
	err = subprocess.Popen(['sudo', 'umount', dev], stderr=subprocess.PIPE).stderr.read()
	if len(err):
		print 'Error:', err
	out = subprocess.Popen(['mountpoint', mpath], stdout=subprocess.PIPE).stdout.read()
	is_mounted = ('is a mountpoint' in out)
	if (is_mounted):
		out = subprocess.Popen(['lsof'], stdout=subprocess.PIPE).stdout.read()
		print '>>> Diagnosing unmount failure...'
		print subprocess.Popen(['lsof'], stdout=subprocess.PIPE).stdout.read()
		print '>>> End diagnosis'
	return (not is_mounted)

def fsAmMount(am, dev, checkdir):
	global scan
	fsAmUnmount(am, dev)
	mpath = os.path.join(self_mount, am.local_path)
	if (not os.path.isdir(mpath)):
		os.makedirs(mpath)
	out = subprocess.Popen(['sudo', 'mount', '-o', 'uid=pi,gid=pi', dev, mpath], stderr=subprocess.PIPE).stderr.read()	
	if (len(out)):
		print 'Error:', out	
	if (scan):
		print 'Scanning (-scan) {}:'.format(mpath), os.listdir(mpath)
	if os.path.isdir(checkdir):
		return True
	print 'Mounting {} failed due to {}'.format(am.filter.descr, checkdir) 
	fsAmUnmount(am, dev)
	return False

def fsAmPowerupCallback(args):
	automounts = args
	cands = fsAmScan(True)
	for am in automounts:
		if (am.filter.enabled and (not am.uuid in cands)):
			return False
	return True

FsAmStatus = namedtuple('FsAmStatus', 'am dev')
def fsAmBegin(automounts):
	fsAmPowerUp(fsAmPowerupCallback, automounts)
	cands = fsAmScan()
	status = {}
	if (len(automounts)):
		print 'Mounting ...',
		for am in automounts:
			if (am.filter.enabled and (am.uuid in cands)):
				dev = cands[am.uuid]
				if (fsAmMount(am, dev, am.filter.dir)):
					status[am.uuid] = FsAmStatus(am, dev)
		print 'Done'
	return status	

def fsAmEnd(automounts, status):
	if (len(status)):
		print 'Unmounting ...',
		for st in status.values():
			if (not fsAmUnmount(st.am, st.dev)):
				return False
		print 'Done'	
	fsAmPowerDown()
	return True

def svnParseOk(err):
	for e in err.splitlines():
		if (not e.lower().startswith('svn: warning:')):
			return False
	return True

def svnGet(url, fpath, silent=False):
	if (not silent):
		print 'svn getting {} {} ...'.format(url, fpath)
	(out, err) = subprocess.Popen(['svn', 'checkout', url, fpath], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False).communicate()
	if (dbgsvn):
		print out if len(out) else '',; err if len(err) else '',;
	return svnParseOk(err)

def svnPut(url, fpath):
	print 'svn putting {} {} ...'.format(url, fpath)
	(out, err) = subprocess.Popen(['svn', 'commit', fpath, '-m', "''"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False).communicate()
	if (dbgsvn):
		print out if len(out) else '',; err if len(err) else '',;
	return svnParseOk(err)

def svnImport(url, fpath):
	print 'svn import {} {} ...'.format(url, fpath)
	(out, err) = subprocess.Popen(['svn', 'import', fpath, url, '-m', "''"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False).communicate()
	if (dbgsvn):
		print out if len(out) else '',; err if len(err) else '',;
	return svnParseOk(err)


FsSessionInfo = namedtuple('FsSessionInfo', 'fs_targets fs_cache_sources max_cache cache_path db_path db_url')
def fsBeginSession():
	global fs_am_status
	global fs_am_targets
	global fs_manual_targets
	global local_cache
	global fs_lcache_sources
	global lcache_size
	fs_am_status = fsAmBegin(fs_ams)
	fs_am_targets = frost.fsFilterMounts(fs_mounts, fs_am_filters, warn, True)
	fs_targets = fs_manual_targets + fs_am_targets
	max_cache = 0; fs_cache_sources = None; cache_path = None;
	if (local_cache):
		 max_cache = lcache_size; fs_cache_sources = fs_lcache_sources; cache_path = self_lcache; 
	else:
		fs_mcache_sources = frost.fsFilterMounts(fs_mounts, fs_mcache_filters, True, True)
		if (len(fs_mcache_sources) > 0):
			source = fs_mcache_sources[0]
			mcache_size = resetCache(source.dir, lim_cache)
			max_cache = mcache_size; fs_cache_sources = fs_mcache_sources; cache_path = source.dir;
		else:
			print 'Could not mount cache.'	
	
	#TODO: include db size in cache
	db_path = None; db_url = None;
	sess_db = arg_db
	if (sess_db is not None):
		if (os.path.isabs(sess_db) and ('http' not in arg_db)):
			db_path = sess_db
		else:	
			db_dir = os.path.join(cache_path, 'db')
			if (not os.path.isdir(db_dir)):	
				os.makedirs(db_dir)
			if ('http' in sess_db):
				splt = sess_db.replace('\\', '/').split('/')
				db_file = splt[-1]
				db_sub = os.path.splitext(db_file)[0]
				url_base = '/'.join(splt[:-1])
				url_dir =  '{}/{}'.format(url_base, db_sub)
				db_url = '{}/{}'.format(url_dir, db_file)
				test_dir = os.path.join(cache_path, db_sub)
				test_path = os.path.join(test_dir, db_file)
				if (not os.path.isdir(test_dir)):	
					os.makedirs(test_dir)
				if (os.path.isdir(test_dir) and svnGet(url_dir, test_dir)):
					db_path = test_path
				elif (db_add):
					if (not os.path.isfile(test_path)):
						with open(test_path, 'a'):
							os.utime(test_path, None)
					if (svnImport(db_url, test_path)):
						os.remove(test_path)
						if svnGet(db_url, test_dir):
							db_path = test_path
			else:
				db_path = os.path.join(cache_path, sess_db)

	return FsSessionInfo(fs_targets, fs_cache_sources, max_cache, cache_path, db_path, db_url)

def fsSessionCloseDb(sess_info):
	if (sess_info.db_path is not None) and (sess_info.db_url is not None):
		return svnPut(sess_info.db_url, sess_info.db_path) 
	else:
		return True

def fsEndSession():
	global fs_am_status
	ret = fsAmEnd(fs_ams, fs_am_status)
	fs_am_status = {}
	return ret

def Linux_ipAddresses():
	ret = []
	out = subprocess.Popen(['ip', 'addr', 'show'], stdout=subprocess.PIPE).stdout.read()
	for line in iter(out.splitlines()):
		data = line.split()
		if (data[0] == 'inet'):
			ip = data[1].split('/')[0]
			if (ip not in ret):
				ret.append(ip)
	if (len(ret) > 1 and '127.0.0.1' in ret):
		ret.remove('127.0.0.1')
	return ret

def _ipAddresses():
	addrList = socket.getaddrinfo(socket.gethostname(), None)
	ipList=[]
	for item in addrList:
		ip = item[4][0]
		print ip
		if (len(ip.split('.')) == 4 and (ip not in ipList)):
			ipList.append(ip)
	return ipList 

def ipAddresses():
	sys = platform.system().lower()
	if ('darwin' in sys):
		return _ipAddresses()
	if ('linux' in sys):
		return Linux_ipAddresses()
	return None	


port = 24107
if ('-port' in sys.argv):
	port = int(sys.argv[int(sys.argv.index('-port')+1)])
address = '127.0.0.1'
if ('-address' in sys.argv):
	address = sys.argv[sys.argv.index('-address')+1]
else:
	ip_list = ipAddresses()
	if dbg3:
		print 'ip_addresses', ip_list
	if (len(ip_list) == 1 or (not use_ui)):
		address = ip_list[0]
	else:
		index = 0
		for ip in ip_list:
			print '{}. {}'.format(index+1, ip); index = index + 1;
		input_str = raw_input('Choose ip: '); choice = int(input_str)-1;
		address = ip_list[choice]

if (dry):
	if (scan):
		am_scan = fsAmScan()
		print 'Scanned (-scan): ', am_scan
	fs_session_info = fsBeginSession()
	print 'Automounted (-dry):', [t.dir for t in fs_session_info.fs_targets], ', cache: {} ({})'.format(fs_session_info.cache_path, niceBytes(fs_session_info.max_cache)), ', db: {} {})'.format(fs_session_info.db_path, fs_session_info.db_url) 
	fsEndSession()
	exit(0)
else:
	if (not no_am):
		print 'Test Scanning Automounts...', fsAmScan(True)

def sockBind(sock, address, port, tries):
	for i in range(tries):
		try:
			sock.bind((address, port+i))
			return port+i
		except socket.error as e:
			if (e.args[0] != errno.EADDRINUSE) and (e.args[0] != errno.EACCES):
				print traceback.format_exc()
				return -1
	return -1

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#print 'Binding to {}:{}'.format(address, port)
port = sockBind(sock, address, port, 10)
if (port < 0):
	print 'Failed to bind'
	exit(0)

main_thread = threading.current_thread()

def discoveryRun(dport, port, msg):
	ANY = '0.0.0.0'
	MCAST_ADDR = '224.168.2.9'
	MCAST_PORT = dport
	sys.stdout.write('Discovery on port {} ...\n'.format(MCAST_PORT))
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
	sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
	sock.bind((ANY,MCAST_PORT))
	sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 255)
	status = sock.setsockopt(socket.IPPROTO_IP,
	socket.IP_ADD_MEMBERSHIP,
	socket.inet_aton(MCAST_ADDR) + socket.inet_aton(ANY));

	sock.setblocking(0)
	ts = time.time()
	while main_thread.isAlive():
		try:
			time.sleep(1)
			data, addr = sock.recvfrom(1024)
		except socket.error, e:
			pass
		else:
			print 'Discovery request from', addr, data
			if (data == '/mediafrost:discover'):
				sock.sendto(msg, addr)

if ('-no_discovery' not in sys.argv and (len(address) != 0 and address != '0.0.0.0')):
	dport = 1600
	if ('-dport' in sys.argv):
		dport = int(sys.argv[int(sys.argv.index('-dport')+1)])
	threading.Thread(target = discoveryRun, args = (dport, port, '{}:{}'.format(address, port))).start()
	#time.sleep(1)
sock.listen(1)


def recvConnBytes(conn, bufsize, totalsize, sleep, processFunc):	
	leftsize = totalsize
	serving = True
	if (dbg2):
		print ' >dbg2',
	while serving and (leftsize > 0):
		try:
			recvsize = min(leftsize, bufsize)
			bytes = conn.recv(recvsize)
			if not bytes:
				serving = False
				conn.close()
				break
		except socket.error as e:
			if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
				if (sleep > 0):
					time.sleep(sleep)
				continue
			else:
				serving = False
				conn.close()
				break
		#print ' >dbg1',  (recv[:128] + '..') if len(recv) > 128 else recv
		processFunc(conn, bytes)
		leftsize = leftsize - len(bytes)
		if (dbg2):
			print '{} ({}),'.format(leftsize, len(bytes)),
	if (dbg2):
		print ''



def recvProcessWriteFile(fileOut):
    recvProcessWriteFile.file = fileOut
    def write(conn, bytes):
		#print 'writing', len(bytes)
		recvProcessWriteFile.file.write(bytes)
    return write


session_count = 0
while 1:
	
	lcache_size = 0
	if (not fsEndSession()):
		print 'Failed to end session.'
		exit(0)
	if (local_cache):
		lcache_size = resetCache(self_lcache, lim_cache)
		if (lcache_size <= 0):
			print 'Not enough local cache space to continue.'
			exit(0)		

	sys.stdout.write('Listening on {}:{} ...\n'.format(address, port))
	sock.setblocking(1)
	conn, addr = sock.accept()

	print 'Connected ' + addr[0] + ':' + str(addr[1])
	cmd_start = '/start'
	cmd_end = '/end'
	cmd_fid = '/fid:'
	fid_len = 256/4
	cmd_fdata = '/fdata:'
	cmd_fidend = '/fidend'
	cmd_fdataend = '/fdataend'
	conn_buf = ''
	serving = True
	sock.setblocking(0)
	
	did_print_file_size = False
	session = None
	session_fid = []
	session_fname = []
	session_fsize = []
	session_fi_lists = []
	session_request_fid = {}
	session_count = session_count + 1

	fs_session_info = fsBeginSession()

	session_bootstrap = ('-bootstrap' in sys.argv) or ((fs_session_info.db_path is not None) and (os.path.getsize(fs_session_info.db_path) <= 8))

	print 'Session targets:', [t.dir for t in fs_session_info.fs_targets]
	print 'Session cache: {} ({})'.format(fs_session_info.cache_path, niceBytes(fs_session_info.max_cache))	
	print 'Session db: {} ({}) {}'.format(fs_session_info.db_path, niceBytes(os.path.getsize(fs_session_info.db_path)), '(bootstrap)' if session_bootstrap else '')

	if (fs_session_info.max_cache == 0) or (fs_session_info.db_path is None):
		print 'Invalid session'
		serving = False
		conn.close()

	#conn.sendall('/info:{}'.format(self_cache_size))
	while serving:
		try:
			recv = conn.recv(64*1024)
			if not recv:
				serving = False
				conn.close()
				break
		except socket.error as e:
			if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
				time.sleep(1)
				continue
			else:
				serving = False
				conn.close()
				break

		if (dbg):
			if (not conn_buf.startswith(cmd_fdata)):
				print ' >dbg1',  (recv[:128] + '..') if len(recv) > 128 else recv
				print ' >dbg2',  (conn_buf[:128] + '..') if len(conn_buf) > 128 else conn_buf
		conn_buf = conn_buf + recv

		recurse_process = True
		while recurse_process:

			if (conn_buf.startswith(cmd_start)):
				cmd_splt = conn_buf.split(':', 3-1)
				if ((len(cmd_splt) == 3) and (cmd_splt[-1] == 'eoc')):
					print 'Recieving fids...'
					target_miss_count = 0
					if (len(cmd_splt[1])):
						req_targets = cmd_splt[1].split(',')
						print 'Requested targets:', req_targets
						for t in req_targets:
							tl = t.lower()
							found = None
							for ct in fs_session_info.fs_targets:
								if (ct.filter.id.lower() == tl) or (ct.filter.descr.lower() == tl):
									found = ct; break;
							if (found is None):
								target_miss_count = target_miss_count+1
								print 'Missing requested target:', t
					if (target_miss_count == 0):
						conn.sendall('/go')
					else:
						conn.send('/frequestend'); conn.send('/success:{}'.format('2'));
						conn.sendall(cmd_end); serving = False; conn.close();
					cmd_len = sum(len(x)+1 for x in cmd_splt)-1
					conn_buf = conn_buf[cmd_len:]
				else:
					recurse_process = False

			elif (conn_buf.startswith(cmd_end)):
				conn_buf = conn_buf[len(cmd_end):]
				conn.close()
				serving = False

				if (session is not None):
					frost.bkpEndSession(session)
					session = None

			elif (conn_buf.startswith(cmd_fid)):
				cmd_splt = conn_buf.split(':', 3)
				if (len(cmd_splt) == 4):
					cmd_hdr_size = len(cmd_splt[0]) + 1 + len(cmd_splt[1]) + 1 + len(cmd_splt[2]) + 1  
					total_len = cmd_hdr_size + fid_len
					if (len(conn_buf) >= total_len):
						cmd_data = conn_buf[cmd_hdr_size:total_len]
						conn_buf = conn_buf[total_len:]	
						fid = cmd_data
						file_name = cmd_splt[1]
						file_size = int(cmd_splt[2])
						if (perfile):
							print 'fid {} {} {}'.format(file_name,file_size,fid)
						session_fid.append(fid); session_fname.append(file_name); session_fsize.append(file_size);
				else:
					recurse_process = False

			elif (conn_buf.startswith(cmd_fidend)):
				print 'Analyzing fids...'
				conn_buf = conn_buf[len(cmd_fidend):]
	
				findices = frost.fiGenUniqueIndices(session_fname)
	

				session_name = 'mediafrost_remote_{}'.format(session_count)
				targets = fs_session_info.fs_targets
				no_db = ('-no_db' in sys.argv)
				if (not no_db):
					session = frost.bkpStartSession(fs_session_info.db_path, session_bootstrap, session_name)
				else:
					session = frost.bkpStartSession(None, True, session_name)
				frost.bkpPrepareTargetTables(session, targets)
	
				target_tbls = []
				fi_lists = session_fi_lists
	
				for target in targets:
					target_tbls.append(frost.dbMakePointFidTableName(target.filter.id))
					fi_lists.append([])

				session_needed_cache = 0
				for fid,fname,fsize,fuind in zip(session_fid, session_fname, session_fsize, findices):
					test_needed_cache = session_needed_cache + fsize	
					funame = frost.fiGenUniqueName(fname, fuind)	
					for target,tbl,fi_list in zip(targets, target_tbls, fi_lists):
						file_exists = frost.bkpExistsFileId(session, fid, tbl)
						if (not file_exists):
							session_needed_cache = session_needed_cache + fsize	
							reject = (session_needed_cache > fs_session_info.max_cache)
							if (reject):
								rel = '-(rejected)->'
							else:	
								fpath = os.path.join(fs_session_info.cache_path, funame)
								session_request_fid[fid] = fpath
								fi_list.append(frost.NewFileInfo(fpath, fid, 0))
								rel = '--->'
						else:
							rel = 'in'
						if (fid in session_request_fid):
							session_needed_cache = test_needed_cache
						if (perfile):	
							print '{} {} {}'.format(funame, rel, target.filter.long_descr)
	
				if (len(session_request_fid) > 0):
					print 'Requesting files...'	
					for fid in session_request_fid.iterkeys():
						if (perfile):
							print 'Requesting {}...'.format(fid)
						conn.send('/frequest:{}'.format(fid))
					conn.sendall('/frequestend')	
					print 'Caching files...'
				else:
					print 'Nothing to do'
					serving = False
					conn.send('/frequestend'); conn.sendall('/success:1');
					conn.close()
					
			elif (conn_buf.startswith(cmd_fdata)):
				cmd_splt = conn_buf.split(':', 3)
				if (len(cmd_splt) == 4):
					file_size = int(cmd_splt[2])
					if (not did_print_file_size):	
						if (perfile):
							print 'reading {} file bytes ...'.format(file_size)
						did_print_file_size = True
					cmd_hdr_size = len(cmd_splt[0]) + 1 + len(cmd_splt[1]) + 1 + len(cmd_splt[2]) + 1 
					total_len = cmd_hdr_size + file_size
					avail_len = len(conn_buf)
					file_fid = cmd_splt[1]
					file_path = session_request_fid[file_fid]
					with open(file_path, 'wb') as output:
						cmd_data = conn_buf[cmd_hdr_size:total_len]
						output.write(cmd_data)
						if (avail_len >= total_len):
							conn_buf = conn_buf[total_len:]
						else:
							conn_buf = ''
							leftsize = total_len - avail_len
							func = recvProcessWriteFile(output)
							recvConnBytes(conn, 64*1024, leftsize, 0, func)
	
						if (perfile):
							print 'wrote', file_name
						did_print_file_size = False
				else:
					process_recurse = False
	
	
			elif (conn_buf.startswith(cmd_fdataend)):
				print 'Backing up ...'	
				conn_buf = conn_buf[len(cmd_fdataend):]
	
				targets = fs_session_info.fs_targets
				source = fs_session_info.fs_cache_sources[0]
				nfi_dict = {}
				for target,list in zip(targets, session_fi_lists):
					nfi_dict[(source, target)] = list
	
				try:
					if ('-dry_target_write' in sys.argv):
						print 'Skipping target writes (-dry_target_write)'
						nfi_dict = {}
					success = frost.bkpBackupFs(session, fs_session_info.fs_cache_sources, targets, nfi_dict)
					frost.bkpEndSession(session)
					session = None
					success = success and fsSessionCloseDb(fs_session_info)
				except:
					print traceback.format_exc()
					success = False
	
				if (success):
					code = 1
				else:
					code = 0
				print 'Success: {}'.format(code)	
				
				conn.send('/success:{}'.format(code))
				conn.sendall(cmd_end)
				serving = False
				conn.close()
			
			else:
				recurse_process = False
				if (dbg and len(conn_buf) > 0):
					print ' >dbg?', (conn_buf[:75] + '..') if len(conn_buf) > 75 else conn_buf
		
	if (session is not None):
		frost.bkpEndSession(session)
		session = None

	conn.close()
	
conn.close()
sock.close()
