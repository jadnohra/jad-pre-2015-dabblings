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

def getFsMountPath(mount):
	info = subprocess.Popen(['mount'], stdout=subprocess.PIPE).stdout.read()
	for line in iter(info.splitlines()):
		data = line.split()
		if data[0] == mount:
			return data[2]
	return None	


def findFsMountedSources(filters):
	found = []
	ret = []
	out = subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()
	disk_name = ''
	for line in iter(out.splitlines()):
		if line.startswith('/'):
			disk_name = line
		else:
			for i in range(len(filters)):
				if (filters[i].enabled and (filters[i].name in line)):
					if i in found:
						print 'Duplicate mount points!'
						return []
					data = line.split()
					mount_name = '/dev/' + data[-1]
					mount_path = getMountPath(mount_name)
					found.append(i)
					ret.append(MountPoint(i, filters[i], disk_name, mount_name, mount_path))
	return ret

FsMountPointFilter = namedtuple('FsMountPointFilter', 'enabled id descr serial name size')
fs_source_filters = [ 
					FsMountPointFilter(True, '-1', 'Canon', '0', 'EOS_DIGITAL', '16'), 
					FsMountPointFilter(True, '-2', 'Panasonic', '0', 'CAM_SD', '32'),
					FsMountPointFilter(False, '-3', 'Apple_HFS', '0', 'Mac', '790'), 
				]
FsMountPoint = namedtuple('FsMountPoint', 'filter_index filter disk mount path')


fs_sources = findFsMountedSources(fs_source_filters)
#fs_sources.insert(0, FsMountPoint(0, fs_source_filters[0], 'dummy', 'dummy', self_dir))
fs_sources.insert(0, FsMountPoint(0, fs_source_filters[0], 'test', 'self_test_in', self_test_in))
print 'fs_sources', fs_sources


################################
def findFtpActiveSources(filters):
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


FtpMountPoint = namedtuple('FtpMountPoint', 'enabled id descr address port user pwd')
ftp_source_filters = [
				FtpMountPoint(True, '-1', 'MacTestFtp', '127.0.0.1', 21, 'jad', '')
		]

# sudo -s launchctl [un]load -w /System/Library/LaunchDaemons/ftp.plist
ftp_sources = findFtpActiveSources(ftp_source_filters)
print 'ftp_sources', ftp_sources

################################
def genFileMD5(fname):
	return hashlib.sha256(open(fname, 'rb').read()).hexdigest()

def genFileCrc32(fileName):
	prev = 0
	for eachLine in open(fileName,"rb"):
		prev = zlib.crc32(eachLine, prev)
	return "%X"%(prev & 0xFFFFFFFF) 


BackupSession = namedtuple('BackupSession', 'dbPath dbConn')
BackupFileInfo = namedtuple('BackupFileInfo', 'fid')
NewFileInfo = namedtuple('NewFileInfo', 'fpath fid')

media_extensions = ['jpg']
def bkpIsMediaFile(name):
	global media_extensions
	return (name.split('.')[-1].lower() in media_extensions)


def bkpGenFileId(fname):
	return str(genFileMD5(fname))

def bkpFindFileId(session, fid):
	t = (fid,)
	c = session.dbConn.execute('SELECT * FROM file_infos WHERE fid=?', t)
	rec = c.fetchone()
	if (rec is None):
		return None
	return BackupFileInfo(fid)

def bkpExistsFile(session, fname):
	fid = bkpGenFileId(fname)
	return bkpFindFileId(session, fid) is not None

def bkpExistsFileId(session, fid):
	return bkpFindFileId(session, fid) is not None

def bkpStartSession(dbPath, bootstrap = False):
	if (bootstrap):
		os.remove(dbPath)	
	else:	
		if (not os.path.isfile(dbPath)):
			return None

	conn = sqlite3.connect(dbPath)
	if conn is None:
		return None
	
	if bootstrap:
		conn.execute('CREATE TABLE file_infos(fid text PRIMARY KEY, bla text)')
	
		perf_test = False
		if perf_test:
			start = time.time()
			for i in range(1000*1000):
				t = (str(uuid.uuid4()), str(uuid.uuid4()),)
				conn.execute("INSERT INTO file_infos VALUES (?,?)",t)
			conn.commit()
			print time.time()-start	
			
		conn.execute("INSERT INTO file_infos VALUES (?,?)", ( bkpGenFileId(self_test_image), 'hey',))
		conn.commit()

		if perf_test:
			session = BackupSession(dbPath, conn)
			start = time.time()
			for i in range(1000):
				bkpFindFileId(session, str(uuid.uuid4()))		
			print time.time()-start		
		
	session = BackupSession(dbPath, conn)
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

def bkpFindNewFsFileInfos(session, mount):
	ret = []
	for subdir, dirs, files in os.walk(mount.path):
		for file in files:
			if (bkpIsMediaFile(file)):
				#print subdir+'/'+file
				fp = os.path.join(subdir,file)
				fid = bkpGenFileId(fp)
				bkpinfo = bkpExistsFileId(session, fid)
				#print fp, not exists
				if not bkpinfo:
					ret.append(NewFileInfo(fp, fid))
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

# http://www.withoutthesarcasm.com/using-amazon-glacier-for-personal-backups/
# duplicator for db file[s]
# multiple outs, glacier should NOT be the only target. http://www.daemonology.net/blog/2012-09-04-thoughts-on-glacier-pricing.html


if False:
	print genFileMD5(self_test_image)
	print genFileCrc32(self_test_image)
	print genFileMD5(self_test_image2)
	print genFileCrc32(self_test_image2)

session = bkpStartSession(self_test_db, False)

if False:
	print bkpExistsFile(session, self_test_image)
	print bkpExistsFile(session, self_test_image2)
bkpDumpDb(session, self_test_db+'.sql')

if (True and len(fs_sources) > 0):
	nfi = bkpFindNewFsFileInfos(session, fs_sources[0])
	print nfi
	bkpBackupNewFileInfos2(session, nfi)
	print bkpFindNewFsFileInfos(session, fs_sources[0])


bkpEndSession(session)

