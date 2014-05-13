import os, sys, inspect
import inspect
from collections import namedtuple
import subprocess
import hashlib
import zlib
import sqlite3
import uuid
import time
import ftplib
import tarfile
import shutil
import datetime
import math

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_module_dir = os.path.join(self_dir, 'modules')
self_test_image2 = os.path.join(self_dir, 'photo_swing.jpg')
self_test_image = os.path.join(self_dir, 'IMG_2733.JPG')
self_test_db = os.path.join(self_dir, 'test.db')
self_test_out = os.path.join(self_dir, 'test/out')
self_test_in = os.path.join(self_dir, 'test/in')

######################################
#sys.path.append(os.path.join(self_module_dir, 'exif-py'))
#import exifread

#f = open(self_test_image, 'rb')
#tags = exifread.process_file(f)
#print tags

#fields = ['date', 'model', 'manufacturer']

#if False:
#	for (key,tag) in tags.iteritems():
#		for field in fields:
#			if field in key.lower():
#				print key,tag

#####################################
def printDiskInfo():
	print subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()

def fsGetMountPath(mount):
	info = subprocess.Popen(['mount'], stdout=subprocess.PIPE).stdout.read()
	for line in iter(info.splitlines()):
		data = line.split()
		if data[0] == mount:
			return data[2]
	return None	


def fsFindMounts():
	print 'Finding file system mounts...'
	
	ret = []
	out = subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()
	disk_name = ''
	for line in iter(out.splitlines()):
		if line.startswith('/'):
			disk_name = line
		else:
			data = line.split()
			mount_name = '/dev/' + data[-1]
			mount_path = fsGetMountPath(mount_name)
			if (mount_path != None):
				ret.append(FsMountPoint(disk_name, mount_name, mount_path, line))
	return ret


def fsFilterMounts(mounts, filters):
	print 'Filtering file system mounts...'

	ret = []
	for i in range(len(filters)):
		if (filters[i].enabled):
			matching_mounts = []
			for m in mounts:
				if (filters[i].mount_match in m.info):
					matching_mounts.append(m)
			if (len(matching_mounts) >= 2):
					print 'Duplicate mount matches!'
					return []
			if (len(matching_mounts) == 1):
				chosen_mount = matching_mounts[0]
				dir = os.path.join(chosen_mount.path, filters[i].dir)
				if (os.path.isdir(dir)):
					ret.append(FsStoragePoint(i, filters[i], chosen_mount.disk, chosen_mount.mount, chosen_mount.path, filters[i].dir))
				else:
					print 'Missing directory:', dir 
	return ret
			
def fsGetPointPath(point):
	return os.path.join(point.mount_path, point.dir)



FsMountPointFilter = namedtuple('FsMountPointFilter', 'enabled id descr long_descr mount_match size dir')
fs_source_filters = [ 
					FsMountPointFilter(True, 'Cam_Canon_Eos550D_1', 'Eos550D','Eos550D', 'EOS_DIGITAL', '16', ''), 
					FsMountPointFilter(True, 'Vid_Panasonic_HDCSD90_1', 'Panasonic','Video', 'CAM_SD', '32', ''),
					FsMountPointFilter(True, 'test_fs_in', 'test_fs_in','test_fs_in', 'Apple_HFS', '790', 'Users/nohra/Jad/mediafrost/tools/test/in'), 
					FsMountPointFilter(True, 'test_fs_in1', 'test_fs_in1','test_fs_in1', 'Apple_HFS', '790', 'Users/nohra/Jad/mediafrost/tools/test/in1'), 

				]
fs_target_filters = [ 
					FsMountPointFilter(True, 'test_fs_out', 'test_fs_out','test_fs_out', 'Apple_HFS', '790', 'Users/nohra/Jad/mediafrost/tools/test/out'), 
					FsMountPointFilter(True, 'test_fs_out1', 'test_fs_out1','test_fs_out1', 'Apple_HFS', '790', 'Users/nohra/Jad/mediafrost/tools/test/out1'),
					FsMountPointFilter(True, 'test_fs_out2', 'test_fs_out2','test_fs_out2', 'Apple_HFS', '790', 'Users/nohra/Jad/mediafrost/tools/test/out2'), 
					FsMountPointFilter(True, 'fs_back1', 'Vault_Jad','Vault_Jad', 'VAULT_JAD', '1000', 'Vault/mediafrost'), 
					FsMountPointFilter(True, 'fs_back2', 'Vault_Lena','Vault_Lena', 'VAULT_LENA', '1000', 'Vault/mediafrost'), 
				]

FsMountPoint = namedtuple('FsMountPoint', 'disk mount path info')
FsStoragePoint =  namedtuple('FsStoragePoint', 'filter_index filter mount_disk mount_mount mount_path dir')

if False:
	fs_mounts = fsFindMounts()
	fs_sources = fsFilterMounts(fs_mounts, fs_source_filters)
	fs_targets = fsFilterMounts(fs_mounts, fs_target_filters)

if False:
	print 'fs_mounts', fs_mounts
	print 'fs_sources', fs_sources
	print 'fs_targets', fs_targets


################################
def ftpFindActiveSources(filters):
	ret = []
	for f in filters:
		try:
			con = ftplib.FTP(); 
			con.connect(f.address, f.port, 0.1)
			con.login(f.user, f.pwd)
			welcome = con.getwelcome()
			if True:
				print welcome
				print con.nlst()
			ret.append(f)
		except ftplib.all_errors, e:
			e
	return ret		


FtpStoragePoint = namedtuple('FtpStoragePoint', 'enabled id descr long_descr address port user pwd')
ftp_source_filters = [
				FtpStoragePoint(True, 'test_ftp', 'test_ftp_jad_motog','test_ftp_jad_motog', '192.168.1.9', 1024, 'mediafrost', 'mediafrost')
		]

# sudo -s launchctl [un]load -w /System/Library/LaunchDaemons/ftp.plist
if False:
	ftp_sources = ftpFindActiveSources(ftp_source_filters)
	if True:
		print 'ftp_sources', ftp_sources

################################
def genFileMD5(fname):
	return hashlib.sha256(open(fname, 'rb').read()).hexdigest()

def genFileCrc32(fileName):
	prev = 0
	for eachLine in open(fileName,"rb"):
		prev = zlib.crc32(eachLine, prev)
	return "%X"%(prev & 0xFFFFFFFF) 


BackupSession = namedtuple('BackupSession', 'dbPath dbConn descr options')
BackupFileInfo = namedtuple('BackupFileInfo', 'fid')
NewFileInfo = namedtuple('NewFileInfo', 'fpath fid fsize')
PointBackupSession = namedtuple('PointBackupSession', 'success impl')


def fiGetFilename(fi):
	head, tail = os.path.split(fi.fpath)
	return tail

media_extensions = ['jpg','mp4','m4v','mov','mts']
def bkpIsMediaFile(name):
	global media_extensions
	return (name.split('.')[-1].lower() in media_extensions)


def bkpGenFileId(fname):
	return str(genFileMD5(fname))
	#return str(genFileCrc32(fname))

def bkpFindFileId(session, fid, tblname):
	t = (fid,)
	c = session.dbConn.execute('SELECT * FROM {} WHERE fid=?'.format(tblname), t)
	rec = c.fetchone()
	if (rec is None):
		return None
	return BackupFileInfo(fid)

def bkpExistsFile(session, fname, tblname):
	fid = bkpGenFileId(fname)
	return bkpFindFileId(session, fid, tblname) is not None

def bkpExistsFileId(session, fid, tblname):
	return bkpFindFileId(session, fid, tblname) is not None

def dbMakePointFidTableName(pointName):
	return 'point_fids_{}'.format(pointName)

def dbAddPointFidTable(conn, tableName):
	conn.execute('CREATE TABLE IF NOT EXISTS {}(fid TEXT PRIMARY KEY, size INTEGER)'.format(tableName))

def dbAddPointFid(conn, tableName, fid, size):
	conn.execute('INSERT INTO {} VALUES (?,?)'.format(tableName), (fid,size,))

def dbBootstrap(conn):
	conn.execute('CREATE TABLE file_infos(fid TEXT PRIMARY KEY, size INTEGER)')
	conn.execute('CREATE TABLE sessions(sind INTEGER PRIMARY KEY AUTOINCREMENT, descr TEXT, state INTEGER)')
	conn.commit()

def bkpStartSession(dbPath, bootstrap = False, descr = ''):
	if (bootstrap):
		os.remove(dbPath)	
	else:	
		if (not os.path.isfile(dbPath)):
			return None

	conn = sqlite3.connect(dbPath)
	if conn is None:
		return None
	
	if bootstrap:
		dbBootstrap(conn)
		
	options = { 'bootstrap':bootstrap }	
	session = BackupSession(dbPath, conn, descr, options)
	return session

def bkpEndSession(session):
	if session is None:
		return
	if (session.dbConn is not None):
		session.dbConn.close()

def bkpDumpDb(session, fp):
	with open(fp, 'w') as f:
		for line in session.dbConn.iterdump():
			f.write('%s\n' % line)

def bkpSessionOption(session, opt, dft = False):
	if (opt in session.options):
		return session.options[opt]
	return dft	

def bkpPrepareTargetTables(session, targets):
	for target in targets:
		point_tbl = dbMakePointFidTableName(target.filter.id)
		dbAddPointFidTable(session.dbConn, point_tbl)
	session.dbConn.commit()


def bkpFindNewFsFileInfos(session, sources, targets):
	dups = {}
	ret = {}
	target_tbls = []
	perfile = bkpSessionOption(session, 'perfile')

	bkpPrepareTargetTables(session, targets)

	for target in targets:
		target_tbls.append(dbMakePointFidTableName(target.filter.id))

	for source in sources:
		root_dir = fsGetPointPath(source)
		
		print 'Analyzing source [{}]...'.format(source.filter.long_descr)

		fi_lists = []
		for target in targets:
			fi_list = []
			ret[(source, target)] = fi_list
			fi_lists.append(fi_list)

		for subdir, dirs, files in os.walk(root_dir):
			for file in files:
				if (bkpIsMediaFile(file)):
					fp = os.path.join(subdir,file)
					fid = bkpGenFileId(fp)
					if (perfile):
						print '[{}] : [{}]'.format(fp, fid)
					is_dup = (fid in dups)
					if (is_dup):
						print 'Found duplicate: [{}] <-> [{}]'.format(dups[fid], fp)
					else:	
						for target,tbl,fi_list in zip(targets, target_tbls, fi_lists):
							if (not bkpExistsFileId(session, fid, tbl)):
								fi_list.append(NewFileInfo(fp, fid))
								dups[fid] = fp
	return ret


def fiGenUniqueIndices(fnames):
	counts = {}
	for fname in fnames:
		if (fname in counts):
			counts[fname] = counts[fname] + 1
		else:
			counts[fname] = 1
	indices = {}
	ret = []
	for fname in fnames:
		index = None
		if (counts[fname] > 1):
			if (fname in indices):
				index = indices[fname] + 1
			else:	
				index = 1
			infices[fname] = index
		ret.append(index)
	return ret

def fiGenUniqueName(fname, index):
	if (not index is None):
		return str(index) + '_' + fname
	return fname


def fsBackupFiles(session, finfos, targetPoint, bkpDir):
	print 'Writing to [{}]...'.format(targetPoint.filter.long_descr)

	point_path = fsGetPointPath(targetPoint)
	bkp_path = os.path.join(point_path, bkpDir) 

	os.makedirs(bkp_path)

	#if (not os.path.isdir(bkp_path)):
	#	print 'Missing target', bkp_path
	#	return PointBackupSession(False, bkp_path)

	perfile = bkpSessionOption(session, 'perfile')
	findices = fiGenUniqueIndices([fiGetFilename(x) for x in finfos])

	for i in range(len(finfos)):
		finfo = finfos[i]
		fname = fiGenUniqueName(fiGetFilename(finfo), findices[i])
		shutil.copyfile(finfo.fpath, os.path.join(bkp_path, fname))
		if (perfile):
			print finfo.fpath, '--->',  os.path.join(bkp_path, fname)
	return PointBackupSession(True, bkp_path)


def fsEndBackupFiles(targetPoint, pointSession):
	if (not pointSession.success):
		return
	fp = os.path.join(pointSession.impl, 'mediafrost.txt')
	with open(fp, 'w') as f:
		f.write('ok\n')

def bkpGenSessionDir(now, sind, si, sc):
	ret = "s"+str(sind)
	if sc > 1:
		ret = ret+str(si+1)
	pref = 'Q'+str(int(math.ceil(now.month/3.0)))+'_'+str(now.year)
	return os.path.join(pref, ret)	

def bkpBackupFs(session, sources, targets, manual_nfi_dict = None):

	if (manual_nfi_dict is None):
		nfi_dict = bkpFindNewFsFileInfos(session, sources, targets)
	else:
		nfi_dict = manual_nfi_dict

	has_work = False
	for nfi in nfi_dict.values():
		has_work = has_work or (len(nfi)>0)

	if (not has_work):
		print 'Nothing to do'
		return True

	session.dbConn.execute("INSERT INTO sessions VALUES (?,?,?)", (None, session.descr, 0))
	session.dbConn.commit()

	#cursor = session.dbConn.execute('SELECT max(id) FROM table_name')
	sind_cursor = session.dbConn.execute('SELECT last_insert_rowid()')
	sind = sind_cursor.fetchone()[0]

	now = datetime.datetime.now()
	fs_sessions = []
	has_errors = False
	for si in range(len(sources)):
		source = sources[si]
		for target in targets:
			nfi = nfi_dict[(source, target)]
			if (len(nfi) > 0):
				bkp_dir = bkpGenSessionDir(now, sind, si, len(sources))
				fs_sess = fsBackupFiles(session, nfi, target, bkp_dir)
				fs_sessions.append((target, fs_sess))
				if (fs_sess.success == False):
					print 'Target Failed'
					has_errors = True
					break
		if (has_errors):
			break
	
	if (has_errors):
		print 'Session failed'
		return False

	print 'Updating database...'

	for t,s in fs_sessions:
		fsEndBackupFiles(t,s)

	for source in sources:
		for target in targets:
			nfi = nfi_dict[(source,target)]
			tbl_name = dbMakePointFidTableName(target.filter.id)
			for finfo in nfi:
				dbAddPointFid(session.dbConn, tbl_name, finfo.fid, finfo.fsize)

	session.dbConn.execute("UPDATE sessions SET state=? WHERE sind=?", (1, sind,))
	session.dbConn.commit()

	print 'Session succeeded'
	return True


def bkpUiChooseStoragePoints(fs_sources, fs_targets):

	points = fs_sources + fs_targets
	types = [1]*len(fs_sources) + [2]*len(fs_targets)
	type_switches = { 1:0, 2:0, 0:3, 3:4, 4:0 }

	def display(points, types):
		type_names = ['Disabled', 'Source', 'Target', 'Source*', 'Target*' ]
		index = 1
		for type,point in zip(types,points):
			print '{}. {}: [{}] [{}]'.format(index, type_names[type], point.filter.long_descr, fsGetPointPath(point))
			index = index + 1

	display(points, types)
	input_str = raw_input('Choose index (Enter or 0): ')	
	while(len(input_str) > 0):
		choice = int(input_str)-1
		if (choice == -1):
			types = [0]*len(types)
			input_str = ''
		else:
			types[choice] = type_switches[types[choice]]
			display(points, types)
			input_str = raw_input('Choose index (Enter or 0): ')	

	del fs_sources[:]
	del fs_targets[:]
	for type,point in zip(types,points):
		if (type == 1 or type == 3):
			fs_sources.append(point)
		if (type == 2 or type == 4):
			fs_targets.append(point)
		
	#print fs_sources
	#print fs_targets

def runTestSession():
	fs_mounts = fsFindMounts()
	fs_sources = fsFilterMounts(fs_mounts, fs_source_filters)
	fs_targets = fsFilterMounts(fs_mounts, fs_target_filters)

	test_bootstrap = ('-bootstrap' in sys.argv)
	session = bkpStartSession(self_test_db, test_bootstrap, 'testing')
	session.options['perfile'] = ('-perfile' in sys.argv)

	if (test_bootstrap):
		for t in fs_targets:
			point_path = os.path.join(t.mount_path, t.dir)
			if ('mediafrost/tools/test/out' in point_path):
				for subdir, dirs, files in os.walk(point_path):
					for dir in dirs:
						if ('session' in dir):
							dp = os.path.join(subdir,dir)
							#print dp
							shutil.rmtree(dp)
	
	if (True):
		bkpUiChooseStoragePoints(fs_sources, fs_targets)
		bkpBackupFs(session, fs_sources, fs_targets)

	bkpEndSession(session)

