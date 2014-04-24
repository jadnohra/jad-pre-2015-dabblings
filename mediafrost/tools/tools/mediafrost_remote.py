import socket

port = 24096
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', port))
sock.listen(1)
while 1:
	print 'Listening on ', port, '...'
	sock.setblocking(1)
	conn, addr = sock.accept()

	print 'Connected ' + addr[0] + ':' + str(addr[1])
	cmd_start = '/start'
	cmd_end = '/end'
	cmd_fid = '/fid:'
	fid_len = len('9a9830fc9e45da240b7cc0a5df935c35ca87ed6b56008737fd05e861e1f0d161')
	conn_buf = ''
	serving = True
	sock.setblocking(0)
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
		if (conn_buf.startswith(cmd_fid)):
			total_len = len(cmd_fid)+fid_len
			if (len(conn_buf) >= total_len):
				cmd_data = conn_buf[:total_len]
				conn_buf = conn_buf[total_len:]
				print cmd_data
conn.close()
sock.close()
