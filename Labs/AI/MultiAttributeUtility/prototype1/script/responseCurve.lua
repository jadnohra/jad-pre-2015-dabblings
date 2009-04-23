

ResponseCurve = class(function(a, input)
	a.input = input
end)


IdentityResponseCurve = class(ResponseCurve, function(a)
	Utility.init(a) 
end)

function IdentityResponseCurve:name()
	return ''
end

function IdentityResponseCurve:map(input)
	return input
end


ExpResponseCurve = class(ResponseCurve, function(a, exponent)
	Utility.init(a) 
	a.exponent = exponent
end)

function ExpResponseCurve:name()
	return 'exp' .. tostring(self.exponent)
end

function ExpResponseCurve:map(input)
	return math.pow(input, self.exponent)
end

InvExpResponseCurve = class(ResponseCurve, function(a, exponent)
	Utility.init(a) 
	a.exponent = exponent
end)

function InvExpResponseCurve:name()
	return tostring(self.exponent) .. 'exp-' 
end

function InvExpResponseCurve:map(input)
	return math.pow(self.exponent, -input)
end

-- implement sampled response curve: http://books.google.de/books?id=4f5Gszjyb8EC&pg=PA81&lpg=PA81&dq=AI+response+curve&source=bl&ots=9ASRtsJwmh&sig=l-amw71jk-eP6VP_FaOhJ0y9tfQ&hl=en&ei=fXXmSfrODcjdsgbYkr2VBw&sa=X&oi=book_result&ct=result&resnum=1#PPA79,M1