
Drive = class(Utility, function(a, description, utility)
	Utility.init(a) 
	a.description = description
	a.utility =  utility
end)

function Drive:name()
	return self.description .. ' : ' .. self.utility:name()
end

function Drive:value()
	return self.utility:value()
end
