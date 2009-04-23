
Utility = class(function(a)
end)


EntityVarUtility = class(Utility, function(a, entity, varName)
	Utility.init(a) 
	a.entity = entity
	a.varName = varName
end)

function EntityVarUtility:name()
	return self.entity.name .. '.' .. self.varName
end

function EntityVarUtility:value()
	return self.entity[self.varName]
end

EntityFuncUtility = class(Utility, function(a, entity, funcName)
	Utility.init(a) 
	a.entity = entity
	a.funcName = funcName
end)

function EntityFuncUtility:name()
	local func = self.entity[self.funcName]
	return self.entity.name .. '.' .. self.funcName
end

function EntityFuncUtility:value()
	local func = self.entity[self.funcName]
	return (func)(self.entity)
end

EntityDistanceUtility = class(Utility, function(a, sourceEntity, targetEntity)
	Utility.init(a) 
	a.sourceEntity = sourceEntity
	a.targetEntity = targetEntity
end)

function EntityDistanceUtility:name()
	return 'dist(' .. self.sourceEntity.name .. ', ' .. self.targetEntity.name .. ')'
end

function EntityDistanceUtility:value()
	return self.sourceEntity:getPos():dist(self.targetEntity:getPos())
end

WeightUtility = class(Utility, function(a, sourceUtility, weight)
	Utility.init(a) 
	a.sourceUtility = sourceUtility
	a.weight = weight
end)

function WeightUtility:name()
	return tostring(self.weight) .. ' x (' .. self.sourceUtility:name() .. ')'
end

function WeightUtility:value()
	return self.weight * self.sourceUtility:value()
end

ResponseCurveUtility = class(Utility, function(a, sourceUtility, responseCurve)
	Utility.init(a) 
	a.sourceUtility = sourceUtility
	a.responseCurve = responseCurve
end)

function ResponseCurveUtility:name()
	return self.responseCurve:name() .. '(' .. self.sourceUtility:name() .. ')'
end

function ResponseCurveUtility:value()
	return self.responseCurve:map(self.sourceUtility:value())
end