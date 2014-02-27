import os, sys, inspect
import inspect
from collections import namedtuple
import subprocess

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_module_dir = os.path.join(self_dir, 'modules')
#self_test_image = os.path.join(self_dir, 'photo_swing.jpg')
self_test_image = os.path.join(self_dir, 'IMG_2733.JPG')

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



