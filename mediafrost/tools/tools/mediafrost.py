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

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_module_dir = os.path.join(self_dir, 'modules')
self_test_image2 = os.path.join(self_dir, 'photo_swing.jpg')
self_test_image = os.path.join(self_dir, 'IMG_2733.JPG')
self_test_db = os.path.join(self_dir, 'test.db')
self_test_out = os.path.join(self_dir, 'test/out')
self_test_in = os.path.join(self_dir, 'test/in')

######################################
sys.path.append(os.path.join(self_module_dir, 'exif-py'))
import exifread

f = open(self_test_image, 'rb')
tags = exifread.process_file(f)
#print tags

fields = ['date', 'model', 'manufacturer']

if False:
	for (key,tag) in tags.iteritems():
		for field in fields:
			if field in key.lower():
				print key,tag

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
	ret = []
	for i in range(len(filters)):
		if (filters[i].enabled):
			matching_mounts = []
			for m in mounts:
				if (filters[i].name in m.info):
					matching_mounts.append(m)
			if (len(matching_mounts) >= 2):
					print 'Duplicate mount matches!'
					return []
			if (len(matching_mounts) == 1):
				dir = os.path.join(m.path, filters[i].dir)
				if (os.path.isdir(dir)):
					ret.append(FsStoragePoint(i, filters[i], m.disk, m.mount, m.path, filters[i].dir))
				else:
					print 'Missing directory:', dir 
	return ret
				

FsMountPointFilter = namedtuple('FsMountPointFilter', 'enabled id descr serial name size dir')
fs_source_filters = [ 
					FsMountPointFilter(True, 'Canon1', 'Canon', '0', 'EOS_DIGITAL', '16', ''), 
					FsMountPointFilter(True, 'Panasonic1', 'Panasonic', '0', 'CAM_SD', '32', ''),
					FsMountPointFilter(True, 'test_fs_in1', 'Apple_HFS', '0', 'Mac', '790', 'Users/nohra/Jad/mediafrost/tools/test/in'), 
					FsMountPointFilter(True, 'test_fs_in2', 'Apple_HFS', '0', 'Mac', '790', 'Users/nohra/Jad/mediafrost/tools/test/in1'), 

				]
fs_target_filters = [ 
					FsMountPointFilter(True, 'test_fs_out1', 'Apple_HFS', '0', 'Mac', '790', 'Users/nohra/Jad/mediafrost/tools/test/out'), 
					FsMountPointFilter(True, 'test_fs_out2', 'Apple_HFS', '0', 'Mac', '790', 'Users/nohra/Jad/mediafrost/tools/test/out1'), 
				]

FsMountPoint = namedtuple('FsMountPoint', 'disk mount path info')
FsStoragePoint =  namedtuple('FsStoragePoint', 'filter_index filter mount_disk mount_mount mount_path dir')

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
			ret.append(f)
		except ftplib.all_errors, e:
			e
	return ret		


FtpStoragePoint = namedtuple('FtpStoragePoint', 'enabled id descr address port user pwd')
ftp_source_filters = [
				FtpStoragePoint(True, 'test_ftp', 'MacTestFtp', '127.0.0.1', 21, 'jad', '')
		]

# sudo -s launchctl [un]load -w /System/Library/LaunchDaemons/ftp.plist
ftp_sources = ftpFindActiveSources(ftp_source_filters)
if False:
	print 'ftp_sources', ftp_sources

################################
def genFileMD5(fname):
	return hashlib.sha256(open(fname, 'rb').read()).hexdigest()

def genFileCrc32(fileName):
	prev = 0
	for eachLine in open(fileName,"rb"):
		prev = zlib.crc32(eachLine, prev)
	return "%X"%(prev & 0xFFFFFFFF) 


BackupSession = namedtuple('BackupSession', 'dbPath dbConn descr')
BackupFileInfo = namedtuple('BackupFileInfo', 'fid')
NewFileInfo = namedtuple('NewFileInfo', 'fpath fid')
PointBackupSession = namedtuple('PointBackupSession', 'success impl')


def fiGetFilename(fi):
	head, tail = os.path.split(fi.fpath)
	return tail

media_extensions = ['jpg','mp4','m4v']
def bkpIsMediaFile(name):
	global media_extensions
	return (name.split('.')[-1].lower() in media_extensions)


def bkpGenFileId(fname):
	return str(genFileMD5(fname))

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
	conn.execute('CREATE TABLE IF NOT EXISTS {}(fid TEXT PRIMARY KEY)'.format(tableName))

def dbAddPointFid(conn, tableName, fid):
	session.dbConn.execute('INSERT INTO {} VALUES (?)'.format(tableName), (fid,))

def dbBootstrap(conn):
	conn.execute('CREATE TABLE file_infos(fid TEXT PRIMARY KEY)')
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
			
	session = BackupSession(dbPath, conn, descr)
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

def bkpFindNewFsFileInfos(session, sources, targets):
	dups = {}
	ret = {}
	target_tbls = []

	for target in targets:
		point_tbl = dbMakePointFidTableName(target.filter.id)
		dbAddPointFidTable(session.dbConn, point_tbl)
		target_tbls.append(point_tbl)
	session.dbConn.commit()

	for source in sources:
		root_dir = os.path.join(source.mount_path, source.dir)

		fi_lists = []
		for target in targets:
			fi_list = []
			ret[(source, target)] = fi_list
			fi_lists.append(fi_list)

		for subdir, dirs, files in os.walk(root_dir):
			for file in files:
				if (bkpIsMediaFile(file)):
					#print subdir+'/'+file
					fp = os.path.join(subdir,file)
					fid = bkpGenFileId(fp)
					is_dup = (fid in dups)
					if (is_dup):
						print 'duplicate', dups[fid], ' <-> ', fp
					else:	
						for target,tbl,fi_list in zip(targets, target_tbls, fi_lists):
							if (not bkpExistsFileId(session, fid, tbl)):
								fi_list.append(NewFileInfo(fp, fid))
								dups[fid] = fp
	return ret

def bkpBackupNewFileInfos(session, finfos):
	for finfo in finfos:
		session.dbConn.execute("INSERT INTO file_infos VALUES (?,?)", ( finfo.fid, finfo.fpath,))
		session.dbConn.commit()
		print 'Backed up', finfo.fpath


def bkpBackupNewFileInfos2(session, finfos):

	if (len(finfos) == 0):
		return

	arch_path = os.path.join(self_test_out, str(uuid.uuid4())+'.tar')
	
	while (os.path.isfile(arch_path)):
		arch_path = os.path.join(self_test_out, str(uuid.uuid4())+'.tar')

	open(arch_path, 'a').close()
	print 'Archive', arch_path
	tar = tarfile.open(arch_path, "w")
	for finfo in finfos:
		tar.add(finfo.fpath)
	tar.close()

	for finfo in finfos:
		session.dbConn.execute("INSERT INTO file_infos VALUES (?,?)", ( finfo.fid, finfo.fpath,))
		session.dbConn.commit()
		print 'Backed up', finfo.fpath


def fsBackupFiles(finfos, targetPoint, bkpDir):

	point_path = os.path.join(targetPoint.mount_path, targetPoint.dir)
	bkp_path = os.path.join(point_path, bkpDir) 

	os.mkdir(bkp_path)

	#if (not os.path.isdir(bkp_path)):
	#	print 'Missing target', bkp_path
	#	return PointBackupSession(False, bkp_path)

	for i in range(len(finfos)):
		finfo = finfos[i]
		fname = str(i) + '_' + fiGetFilename(finfo)
		shutil.copyfile(finfo.fpath, os.path.join(bkp_path, fname))
		print finfo.fpath, '--->',  os.path.join(bkp_path, fname)

	return PointBackupSession(True, bkp_path)


def fsEndBackupFiles(targetPoint, pointSession):

	if (not pointSession.success):
		return

	fp = os.path.join(pointSession.impl, 'mediafrost.txt')
	with open(fp, 'w') as f:
		f.write('ok\n')


def bkpBackupFs(session, sources, targets):

	nfi_dict = bkpFindNewFsFileInfos(session, sources, targets)
	
	has_work = False
	for nfi in nfi_dict.values():
		has_work = has_work or (len(nfi)>0)

	if (not has_work):
		print 'no work'
		return

	session.dbConn.execute("INSERT INTO sessions VALUES (?,?,?)", (None, session.descr, 0))
	session.dbConn.commit()

	#cursor = session.dbConn.execute('SELECT max(id) FROM table_name')
	sind_cursor = session.dbConn.execute('SELECT last_insert_rowid()')
	sind = sind_cursor.fetchone()[0]

	fs_sessions = []
	has_errors = False
	for si in range(len(sources)):
		source = sources[si]
		for target in targets:
			nfi = nfi_dict[(source, target)]
			if (len(nfi) > 0):
				bkp_dir = 'session_' + str(sind) + '_' + str(si)
				fs_sess = fsBackupFiles(nfi, target, bkp_dir)
				fs_sessions.append((target, fs_sess))
				if (fs_sess.success == False):
					has_errors = True
					break
		if (has_errors):
			break
	
	if (has_errors):
		return

	for t,s in fs_sessions:
		fsEndBackupFiles(t,s)

	for source in sources:
		for target in targets:
			nfi = nfi_dict[(source,target)]
			tbl_name = dbMakePointFidTableName(target.filter.id)
			for finfo in nfi:
				dbAddPointFid(session.dbConn, tbl_name, finfo.fid)

	session.dbConn.execute("UPDATE sessions SET state=? WHERE sind=?", (1, sind,))
	session.dbConn.commit()

	


# http://www.withoutthesarcasm.com/using-amazon-glacier-for-personal-backups/
# duplicator for db file[s]
# multiple outs, glacier should NOT be the only target. http://www.daemonology.net/blog/2012-09-04-thoughts-on-glacier-pricing.html


if False:
	print genFileMD5(self_test_image)
	print genFileCrc32(self_test_image)
	print genFileMD5(self_test_image2)
	print genFileCrc32(self_test_image2)

test_bootstrap = ('-bootstrap' in sys.argv)
session = bkpStartSession(self_test_db, test_bootstrap, 'testing')

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
	bkpBackupFs(session, fs_sources, fs_targets)


bkpEndSession(session)

