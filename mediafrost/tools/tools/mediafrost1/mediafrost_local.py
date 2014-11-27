
import sys
import os
import traceback
import errno
import time
import platform
import subprocess
import shutil
from sets import Set
from collections import namedtuple
import mediafrost as frost

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
fs_source_filters = []	

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

#FsMountPointFilter = namedtuple('FsMountPointFilter', 'enabled id descr long_descr mount_match size dir')
fs_am_filters = [
				frost.FsMountPointFilter(True, 'fs_back1', 'Vault_Jad','Vault_Jad', default_mount_match_name, '1000', os.path.join(self_mount,'vault_jad/Vault/mediafrost')),
				frost.FsMountPointFilter(True, 'fs_back2', 'Vault_Lena','Vault_Lena', default_mount_match_name, '1000', os.path.join(self_mount,'vault_lena/Vault/mediafrost')),
             ]
fs_am_source_filters = [
				frost.FsMountPointFilter(True, 'fs_cannon1', 'Cannon-EOS-550D','Cannon-EOS-550D', default_mount_match_name, '1000', os.path.join(self_mount,'cannon1')),
             ]             
FsAutoMount = namedtuple('FsAutoMount', 'filter uuid local_path')
fs_ams = [
            FsAutoMount(fs_am_filters[0], '150B-2565', 'vault_jad'),
            FsAutoMount(fs_am_filters[1], '1101-1163', 'vault_lena'),
            FsAutoMount(fs_mcache_filters[0], '60CD-BC6D', 'mcache'),
            FsAutoMount(fs_am_source_filters[0], '??????', 'cannon1'),
        ]
if (no_am):
    fs_am_filters = []; fs_ams = [];
fs_am_status = {}
fs_am_targets = []


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


FsSessionInfo = namedtuple('FsSessionInfo', 'fs_targets fs_sources max_cache cache_path db_path db_url')
def fsBeginSession():
	global fs_am_status
	global fs_am_targets
	global local_cache
	global fs_sources
	global lcache_size
	fs_am_status = fsAmBegin(fs_ams)
	fs_am_targets = frost.fsFilterMounts(fs_mounts, fs_am_filters, warn, True)
	fs_am_sources = frost.fsFilterMounts(fs_mounts, fs_am_source_filters, warn, True)
	fs_targets = fs_am_targets
	fs_sources = fs_am_sources

	max_cache = 0; cache_path = None;
	db_path = None; db_url = None;

	if (len(fs_sources)):	
		if (local_cache):
			 max_cache = lcache_size; cache_path = self_lcache; 
		else:
			fs_mcache_sources = frost.fsFilterMounts(fs_mounts, fs_mcache_filters, True, True)
			if (len(fs_mcache_sources) > 0):
				max_cache = mcache_size; cache_path = source.dir;
			else:
				print 'Could not mount cache.'	
		
		#TODO: include db size in cache
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
	else:
		print 'Did not find any sources.'				

	return FsSessionInfo(fs_targets, fs_sources, max_cache, cache_path, db_path, db_url)

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


if 1:
	
	lcache_size = 0
	if (local_cache):
		lcache_size = resetCache(self_lcache, lim_cache)
		if (lcache_size <= 0):
			print 'Not enough local cache space to continue.'
			exit(0)		

	fs_session_info = fsBeginSession()

	if (len(fs_session_info.fs_sources)):

		session_bootstrap = ('-bootstrap' in sys.argv) or ((fs_session_info.db_path is not None) and (os.path.getsize(fs_session_info.db_path) <= 8))

		print 'Session targets:', [t.dir for t in fs_session_info.fs_targets]
		print 'Session cache: {} ({})'.format(fs_session_info.cache_path, niceBytes(fs_session_info.max_cache))	
		print 'Session db: {} ({}) {}'.format(fs_session_info.db_path, niceBytes(os.path.getsize(fs_session_info.db_path)), '(bootstrap)' if session_bootstrap else '')

		if (fs_session_info.max_cache == 0) or (fs_session_info.db_path is None):
			print 'Invalid session'
			serving = False
			conn.close()
		
		print 'Backing up ...'	

		try:
			success = frost.bkpBackupFs(session, fs_session_info.fs_sources, fs_session_info.fs_targets)
			success = success and fsSessionCloseDb(fs_session_info)
		except:
			print traceback.format_exc()
			success = False
		
		if (success):
			code = 1
		else:
			code = 0
		print 'Success: {}'.format(code)	
			
		frost.bkpEndSession(session)
		session = None

	fsEndSession()	

