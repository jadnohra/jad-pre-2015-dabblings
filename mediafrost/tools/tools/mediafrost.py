import os, sys, inspect
import inspect
from collections import namedtuple
import subprocess
import hashlib
import zlib
import sqlite3

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
def printMountPoints():
	print subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()

def findMountPoints(points):
	found = []
	ret = []
	out = subprocess.Popen(['diskutil', 'list'], stdout=subprocess.PIPE).stdout.read()
	mount_path = ''
	for line in iter(out.splitlines()):
		if line.startswith('/'):
			mount_path = line
		for i in range(len(points)):
			if (points[i].name in line):
				if i in found:
					print 'Duplicate mount points!'
					return []
				found.append(i)
				ret.append([i, points[i], mount_path])
	return ret

MountPoint = namedtuple('MountPoint', 'id descr serial name size')
mount_points = [ MountPoint('-1', 'Canon', '0', 'EOS_DIGITAL', '16'), MountPoint('-2', 'Panasonic', '0', 'CAM_SD', '32')  ]


found_mount_points = findMountPoints(mount_points)
print found_mount_points

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

def bkpGenFileId(fname):
	return str(genFileMD5(fname))

def bkpFindFileId(session, fid):
	t = (fid,)
	c = session.dbConn.execute('SELECT * FROM file_infos WHERE fid=?', t)
	rec = c.fetchone()
	if (rec is None):
		return None
	return BackupFileInfo(fid)

def bkpIsFileNew(session, fname):
	fid = bkpGenFileId(fname)
	return bkpFindFileId(session, fid) is None

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
		conn.execute('CREATE TABLE file_infos(fid text, bla text)')
		conn.commit()

	session = BackupSession(dbPath, conn)
	return session

def bkpEndSession(session):
	if session is None:
		return
	if (session.dbConn is not None):
		session.dbConn.close()

print genFileMD5(self_test_image)
print genFileCrc32(self_test_image)
print genFileMD5(self_test_image2)
print genFileCrc32(self_test_image2)

session = bkpStartSession(self_test_db, True)
print bkpIsFileNew(session, self_test_image)
bkpEndSession(session)

