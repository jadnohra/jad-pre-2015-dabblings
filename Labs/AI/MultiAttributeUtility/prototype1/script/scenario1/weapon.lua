
Weapon = class(function(a)
end)

function Weapon:description()
	return ''
end

Shotgun = class(Weapon, function(a)
	Weapon.init(a) 
	a.rounds = 10
	a.maxRounds = 10
end)

function Shotgun:description()
	return 'Shotgun[' .. self.rounds .. ']'
end