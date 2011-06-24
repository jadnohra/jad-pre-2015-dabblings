import struct
from bitstring import BitArray

def cf32(num):
	bstr = str(f32b(num)) 
	ba = BitArray(bin=bstr)
	return struct.unpack('<f', struct.pack('I', ba.uint))[0]


def bbf32(b0=0b0, b1=0b0, b2=0b0, b3=0b0):
	return cf32(struct.unpack('>f', struct.pack('b', int(b0)) + struct.pack('b', int(b1)) + struct.pack('b', int(b2)) + struct.pack('b', int(b3)))[0])


def sbf32(sb='00000000000000000000000000000000'):
	ba = BitArray(bin=sb)
	return cf32(struct.unpack('<f', struct.pack('I', ba.uint))[0])


def sbf32(exp='00000000', mant='00000000000000000000000', sgn='0'):
	ba = BitArray(bin=sgn+exp+mant)
	return cf32(struct.unpack('<f', struct.pack('I', ba.uint))[0])


def abf32(exp=[], mant=[], sgn='0'):
	exps='00000000'
	mants='00000000000000000000000'
	
	ba = BitArray(bin=sgn+exps+mants)

	for i in exp: 
		ba.set(1, (len(sgn) + len(exps) - i) - 1)
	for i in mant: 
		ba.set(1, (len(sgn) + len(exps) + len(mants) + i) - 1)

	#print ba.uint
	#print struct.unpack('<f', struct.pack('I', ba.uint))
	
	#print ba.bin
	#return ba.floatbe
	return cf32(struct.unpack('<f', struct.pack('I', ba.uint))[0])

def f32b(flt):
	return BitArray('float:32='+`flt`).bin

def bf32test():
	#print `struct.unpack('h', '\xFF\b0000000000000000')`
	#print `struct.unpack('h', '\b002\000')`
	#print 'i':bitarray('0001')
	#print 'i':bitarray('0001')
	#print `bitarray('0001').pack()`
	print hex(0b0011)
	ss = (str(hex(0b0010011))+str(hex(0b0010011))+str(hex(0b010011))+str(hex(0b010011))).replace('0x', '\\x')
	print ss
	print type(ss)
	#ss = '\x13\x13\x13\x13'
	print ss[0]
	print '\x13\x13\x13\x13'[0]
	print ss[2] == '\x13\x13\x13\x13'[2]
	print '\\x{0}'.format(1)
	print type(ss)
	print struct.pack('h', 18)
	print struct.unpack('h', struct.pack('h', 18))
	print struct.unpack('h', struct.pack('h', int(0b01)))
	print struct.unpack('h', struct.pack('b', 18) + struct.pack('b', 18))
	print struct.unpack('i', struct.pack('b', 18) + struct.pack('b', 18) + struct.pack('b', 18) + struct.pack('b', 18))

	print struct.unpack('f', struct.pack('b', 18) + struct.pack('b', 18) + struct.pack('b', 18) + struct.pack('b', 18))
	print struct.unpack('>f', struct.pack('b', int(0b00)) + struct.pack('b', int(0b00)) + struct.pack('b', int(0b00)) + struct.pack('b', int(0b01)))
	print bbf32()
	print bbf32(0b0, 0b0, 0b0, 0b1)
	ba = BitArray(bin='00000000000000000000000000000001')
	print ba.float
	print sbf32()

	arr=[1]
	for i in arr: print i


	print abf32([], [0])
	#print struct.unpack('f', ss)
	#print struct.unpack('f', '\x13\x13\x13\x13')
	#print struct.unpack('f', str(hex(11)))
	#print struct.unpack('f', 12)
	#print bin(12)


#bf32test()
print "\nNumbers:"
print "Smallest fp (denormalized) dec.: " + '{0}'.format(abf32([0],[]))	
print "Smallest fp (denormalized) bin.: " + '{0}'.format(f32b(abf32([0],[])))	
print "Double smallest fp (denormalized) dec.: " + '{0}'.format(abf32([],[0]) + abf32([],[0]))	
print "Smallest fp dec.: " + '{0}'.format(abf32([1],[]))
print "Smallest fp bin.: " + '{0}'.format(f32b(abf32([1],[])))

print "\nMisc.:"
print abf32([0],[]) + abf32([1],[])
print f32b(abf32([0],[]) + abf32([1],[]))
print f32b(1.25)


print "\nPrecision:"
print "\nLargest fp killed by addition to 1:"
fone = abf32([0,1,2,3,4,5,6],[])
feps = abf32([0,1,2,5,6],[])
print `fone` + " + " + `feps` + " = " + `cf32(fone+feps)`
print `f32b(fone)` + " + " + `f32b(feps)` + " = " + `f32b(cf32(fone+feps))`

print "\nSmallest fp killing 1 by addition:"
fone = abf32([0,1,2,3,4,5,6],[])
fkiller = abf32([0,1,2,4,7],[])
print `fone` + " + " + `fkiller` + " = " + `cf32(fone+fkiller)`
print `f32b(fone)` + " + " + `f32b(fkiller)` + " = " + `f32b(cf32(fone+fkiller))`


print "\nSmallest fp that can be added to 1 without dying:"
feps = abf32([0,1,2,5,6],[0])
print `fone` + " + " + `feps` + " = " + `cf32(fone+feps)`
print `f32b(fone)` + " + " + `f32b(feps)` + " = " + `f32b(cf32(fone+feps))`

print "\nLargest fp killed by addition:"
fone = abf32([1,2,3,4,5,6,7],[])
feps = abf32([1,2,5,6,7],[])
print `fone` + " + " + `feps` + " = " + `cf32(fone+feps)`
print `f32b(fone)` + " + " + `f32b(feps)` + " = " + `f32b(cf32(fone+feps))`

print "\nLargest fp not killed by addition to fp_max:"
feps = abf32([1,2,5,6,7],[0])
print `fone` + " + " + `feps` + " = " + `cf32(fone+feps)`
print `f32b(fone)` + " + " + `f32b(feps)` + " = " + `f32b(cf32(fone+feps))`

