import socket
import sys
from sets import Set
import mediafrost as frost

self_path = os.path.realpath(__file__)
self_dir = os.path.dirname(self_path)
self_test_cache = os.path.join(self_dir, 'remotecache')
if (not os.path.isdir(self_test_cache)):
	os.path.mkdir(self_test_cache)

#frost.printDiskInfo()

fs_mounts = frost.fsFindMounts()
fs_sources = []
fs_targets = frost.fsFilterMounts(fs_mounts, frost.fs_target_filters)
if False:
	bkpUiChooseStoragePoints(fs_sources, fs_targets)


port = 24096
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', port))
sock.listen(1)
while 1:
	print 'Listening on', port, '...'
	sock.setblocking(1)
	conn, addr = sock.accept()

	print 'Connected ' + addr[0] + ':' + str(addr[1])
	cmd_start = '/start'
	cmd_end = '/end'
	cmd_fid = '/fid:'
	fid_len = 256/4
	cmd_file = '/fdata:'
	cmd_endfid = '/fidend'
	conn_buf = ''
	serving = True
	sock.setblocking(0)
	
	did_print_file_size = False
	session = None
	session_fid = {}

	while serving:
		try:
			recv = conn.recv(1024)
		except socket.error, e:
			err = e.args[0]
			if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
				sleep(1)
				continue
			else:
				print e
				serving = False
		conn_buf = conn_buf + recv

		if (conn_buf.startswith(cmd_start)):
			print cmd_start
			conn_buf = conn_buf[len(cmd_start):]

		if (conn_buf.startswith(cmd_end)):
			print cmd_end
			conn_buf = conn_buf[len(cmd_end):]
			conn.close()
			serving = False

			if (not session is None):
				frost.bkpEndSession(session)

		if (conn_buf.startswith(cmd_fid)):
			cmd_splt = conn_buf.split(':', 2)
			if (len(cmd_splt) == 3):
				cmd_hdr_size = len(cmd_splt[0]) + 1 + len(cmd_splt[1]) + 1 
				total_len = cmd_hdr_size + fid_len
				if (len(conn_buf) >= total_len):
					cmd_data = conn_buf[cmd_hdr_size:total_len]
					conn_buf = conn_buf[total_len:]	
					fid = cmd_data
					file_name = cmd_splt[1]
					print 'fid',file_name,cmd_data
					session_fid[fid] = file_name

		if (conn_buf.startswith(cmd_endfid)):
			conn_buf = conn_buf[len(cmd_endfid):]

			targets = fs_targets
			test_bootstrap = ('-bootstrap' in sys.argv)
			session = frost.bkpStartSession(frost.self_test_db, test_bootstrap, 'remote_testing')
			frost.bkpPrepareTargetTables(session, targets)

			target_tbls = []
			fi_lists = []
			request_fids = Set()

			for target in targets:
				target_tbls.append(frost.dbMakePointFidTableName(target.filter.id))
				fi_lists.append([])
			
			for fid,fname in session_fid.iteritems():
				for target,tbl,fi_list in zip(targets, target_tbls, fi_lists):
					file_exists = frost.bkpExistsFileId(session, fid, tbl)
					if (not file_exists):
						request_fids.add(fid) 
						fi_list.append(frost.NewFileInfo(fname, fid))
						rel = '--->'
					else:
						rel = 'in'
					print '{} {} {}'.format(fname, rel, target.filter.long_descr)
						
			for fid in request_fids:
				print 'Requesting {}...'.format(fid)
				conn.send('/frequest:{}'.format(fid))
			conn.sendall('/end')	
				
		if (conn_buf.startswith(cmd_file)):
			cmd_splt = conn_buf.split(':', 3)
			if (len(cmd_splt) == 4):
				file_size = int(cmd_splt[2])
				if (not did_print_file_size):	
					print 'reading {} file bytes ...'.format(file_size)
					did_print_file_size = True
				cmd_hdr_size = len(cmd_splt[0]) + 1 + len(cmd_splt[1]) + 1 + len(cmd_splt[2]) + 1 
				total_len = cmd_hdr_size + file_size
				if (len(conn_buf) >= total_len):
					file_name = cmd_splt[1]
					file_path = os.path.join(self_test_cache, file_name)
					cmd_data = conn_buf[cmd_hdr_size:total_len]
					conn_buf = conn_buf[total_len:]
					with open(file_name, 'wb') as output:
						output.write(cmd_data)
					print 'wrote', file_name
					did_print_file_size = False	
		
conn.close()
sock.close()
