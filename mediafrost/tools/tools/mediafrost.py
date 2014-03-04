import os, sys, inspect
import inspect
from collections import namedtuple
import subprocess
import hashlib
import zlib
import sqlite3
import uuid
import time

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_module_dir = os.path.join(self_dir, 'modules')
self_test_image2 = os.path.join(self_dir, 'photo_swing.jpg')
self_test_image = os.path.join(self_dir, 'IMG_2733.JPG')
self_test_db = os.path.join(self_dir, 'test.db')

######################################
sys.path.append(os.path.join(self_module_dir, 'exif-py'))
import exifread

f = open(self_test_image, 'rb')
tags = exifread.process_file(f)
#print tags

fields = ['date', 'model', 'manufacturer']

for (key,tag) in tags.iteritems():
	for field in fields:
		if field in key.lower():
			print key,tag

#####################################
def printDiskInfo():
	print subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()

def getMountPath(mount):
	info = subprocess.Popen(['mount'], stdout=subprocess.PIPE).stdout.read()
	for line in iter(info.splitlines()):
		data = line.split()
		if data[0] == mount:
			return data[2]
	return None	


def findMountedSources(filters):
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

MountPointFilter = namedtuple('MountPointFilter', 'enabled id descr serial name size')
source_filters = [ 
					MountPointFilter(True, '-1', 'Canon', '0', 'EOS_DIGITAL', '16'), 
					MountPointFilter(True, '-2', 'Panasonic', '0', 'CAM_SD', '32'),
					MountPointFilter(False, '-3', 'Apple_HFS', '0', 'Mac', '790'), 
				]
MountPoint = namedtuple('MountPoint', 'filter_index filter disk mount path')


found_sources = findMountedSources(source_filters)
found_sources.insert(0, MountPoint(0, source_filters[0], 'dummy', 'dummy', self_dir))
print found_sources

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

def bkpFindNewFiles(session, mount):
	for subdir, dirs, files in os.walk(mount.path):
		for file in files:
			if (bkpIsMediaFile(file)):
				#print subdir+'/'+file
				fp = os.path.join(subdir,file)
				print fp, not bkpExistsFile(session, fp)
			
	return 0


print genFileMD5(self_test_image)
print genFileCrc32(self_test_image)
print genFileMD5(self_test_image2)
print genFileCrc32(self_test_image2)

session = bkpStartSession(self_test_db, True)
print bkpExistsFile(session, self_test_image)
print bkpExistsFile(session, self_test_image2)
bkpDumpDb(session, self_test_db+'.sql')

if (len(found_sources) > 0):
	bkpFindNewFiles(session, found_sources[0])


bkpEndSession(session)

