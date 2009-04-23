
Entity = class(CircleEntity, function(a, name, x, y, r)
	CircleEntity.init(a, name, x, y, r) 
	a.health = nil
	a.weapon = nil
end)

function Entity:description()
	
	local baseDesc =  self.name
	
	if (self.health ~= nil) then
		baseDesc = baseDesc .. '[' .. tostring(self.health*100) .. '] '
	end
	
	if (self.weapon ~= nil) then
		baseDesc = baseDesc .. self.weapon:description()
	end
	
	return baseDesc
end

function Entity:getHealth()
	return self.health
end

function Entity:getWeaponRoundRatio()

	if self.weapon ~= nil then
		return self.weapon.rounds / self.weapon.maxRounds
	end

	return 1.0
end


