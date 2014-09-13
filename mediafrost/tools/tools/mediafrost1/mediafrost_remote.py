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
import mediafrost as frost

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_cache = os.path.join(self_dir, 'cache')
self_cache_size = 0
self_test_out = os.path.join(self_dir, 'test_out')
self_db = os.path.join(self_dir, 'mediafrost.db')

perfile = ('-perfile' in sys.argv)
dbg = ('-dbg' in sys.argv)
dbg2 =('-dbg2' in sys.argv)
dbg3 = ('-dbg3' in sys.argv)

def reset_cache():
	global self_cache
	global self_cache_size
	if (os.path.isdir(self_cache)):
		shutil.rmtree(self_cache)
	os.makedirs(self_cache)
	statvfs = os.statvfs(self_cache)
	self_cache_size = statvfs.f_frsize*statvfs.f_bavail
	self_cache_size = self_cache_size-1024*1024*128

def nice_bytes(bytes):
	a = ['bytes', 'KB.', 'MB.', 'GB.']
	r = float(bytes); i = 0; 
	while (i<len(a) and  (r/1024.0 > 1.0)):	
		r = r / 1024.0; i=i+1;
	return '{:.2f} {}'.format(r, a[i])

reset_cache()
print '{} of disk cache available.'.format(nice_bytes(self_cache_size))
if self_cache_size <= 1024*1024*256:
	exit(0)

if (not os.path.isdir(self_test_out)):
	os.makedirs(self_test_out)


fs_target_filters = [ frost.FsMountPointFilter(True, 'test_fs_out', 'test_fs_out','test_fs_out', '/dev/root', '0', self_test_out) ]

fs_mounts = frost.fsFindMounts()
if dbg3:
	print 'fs_mounts:', fs_mounts	
fs_sources = []
fs_targets = frost.fsFilterMounts(fs_mounts, fs_target_filters)
if dbg3:
	print 'fs_targets:', fs_targets	
use_ui = ('-ui' in sys.argv)
if use_ui:
	frost.bkpUiChooseStoragePoints(fs_sources, fs_targets)

fs_cache_filters = [ frost.FsMountPointFilter(True, 'fs_cache', 'fs_cache','fs_cache', '/dev/root', '0', self_cache) ]
fs_cache_sources = frost.fsFilterMounts(fs_mounts, fs_cache_filters)


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

if ('-dry' in sys.argv):
	exit(0)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#print 'Binding to {}:{}'.format(address, port)
sock.bind((address, port))

main_thread = threading.current_thread()

def discoveryRun(port, msg):
	ANY = '0.0.0.0'
	MCAST_ADDR = '224.168.2.9'
	MCAST_PORT = 1600	
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
	threading.Thread(target = discoveryRun, args = (port, '{}:{}'.format(address, port))).start()
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
	session_fi_lists = []
	session_request_fid = {}
	session_count = session_count + 1

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
				print 'Recieving fids...'
				conn_buf = conn_buf[len(cmd_start):]

			elif (conn_buf.startswith(cmd_end)):
				conn_buf = conn_buf[len(cmd_end):]
				conn.close()
				serving = False

				if (not session is None):
					frost.bkpEndSession(session)

			elif (conn_buf.startswith(cmd_fid)):
				cmd_splt = conn_buf.split(':', 2)
				if (len(cmd_splt) == 3):
					cmd_hdr_size = len(cmd_splt[0]) + 1 + len(cmd_splt[1]) + 1 
					total_len = cmd_hdr_size + fid_len
					if (len(conn_buf) >= total_len):
						cmd_data = conn_buf[cmd_hdr_size:total_len]
						conn_buf = conn_buf[total_len:]	
						fid = cmd_data
						file_name = cmd_splt[1]
						if (perfile):
							print 'fid',file_name,cmd_data
						session_fid.append(fid); session_fname.append(file_name);
				else:
					recurse_process = False

			elif (conn_buf.startswith(cmd_fidend)):
				print 'Analyzing fids...'
				conn_buf = conn_buf[len(cmd_fidend):]
	
				findices = frost.fiGenUniqueIndices(session_fname)
	

				session_name = 'mediafrost_remote_{}'.format(session_count)
				targets = fs_targets
				no_db = ('-no_db' in sys.argv)
				if (not no_db):
					bootstrap = ('-bootstrap' in sys.argv)
					dbPath = self_db
					if ('-db' in sys.argv):
						dbPath = sys.argv[sys.argv.index('-db')+1]
					if (not os.path.isfile(dbPath)):
						bootstrap = True
					session = frost.bkpStartSession(dbPath, bootstrap, session_name)
				else:
					session = frost.bkpStartSession(None, True, session_name)
				frost.bkpPrepareTargetTables(session, targets)
	
				target_tbls = []
				fi_lists = session_fi_lists
	
				for target in targets:
					target_tbls.append(frost.dbMakePointFidTableName(target.filter.id))
					fi_lists.append([])
	
				for fid,fname,fuind in zip(session_fid, session_fname, findices):
					funame = frost.fiGenUniqueName(fname, fuind)	
					for target,tbl,fi_list in zip(targets, target_tbls, fi_lists):
						file_exists = frost.bkpExistsFileId(session, fid, tbl)
						if (not file_exists):
							fpath = os.path.join(self_cache, funame)
							session_request_fid[fid] = fpath
							fi_list.append(frost.NewFileInfo(fpath, fid, 0))
							rel = '--->'
						else:
							rel = 'in'
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
					conn.sendall('/frequestend')
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
				print 'Backing up...'	
				conn_buf = conn_buf[len(cmd_fdataend):]
	
				source = fs_cache_sources[0]
				nfi_dict = {}
				for target,list in zip(fs_targets, session_fi_lists):
					nfi_dict[(source, target)] = list
	
				try:
					success = frost.bkpBackupFs(session, fs_cache_sources, fs_targets, nfi_dict)
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
		
	conn.close()
	
conn.close()
sock.close()
