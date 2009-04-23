
EntityDriveWidget = class(function(a, entity)
	a.pos = Vector2D(0.0, 0.0)
	a.radius = 0.5
	a.entity = entity
end)

function EntityDriveWidget:contains(point)
	return point:dist(self.pos) <= self.radius
end

function EntityDriveWidget:moveTo(point)
	self.pos = point
end

function EntityDriveWidget:draw(engine, camera)
   
   local renderPos = camera:transform(self.pos)
   
   love.graphics.setColor(self.color)
	
   love.graphics.draw(self.name, renderPos[1], renderPos[2])
   love.graphics.circle(love.draw_line, renderPos[1], renderPos[2], self.radius * camera.scale)
end


function EntityDriveWidget:update(engine, dt)
	
	local mousePoint = engine.camera:invTransform(Vector2D(love.mouse.getX(), love.mouse.getY()))
	self.pickedEntity = pickEntity(engine.entities, mousePoint)
end	

function EntityDriveWidget:draw(engine, camera)

	local renderPos = camera:transform(self.pos)

	love.graphics.setColor(self.entity.color)
	
    love.graphics.circle(love.draw_line, renderPos[1], renderPos[2], self.radius * camera.scale)	
	renderPos[2] = renderPos[2] + 12
	
	-- unsorted
	--for i,drive in ipairs(self.entity.drives) do
	--	love.graphics.draw('#' .. tostring(i) .. '-' .. drive:name() .. ' = ' .. tostring(drive:value()), renderPos[1], renderPos[2])
	--	renderPos[2] = renderPos[2] + 12
	--end
	
	local sortedDrives = {}
	
	for i,drive in ipairs(self.entity.drives) do
		sortedDrives[i] = drive
	end
	
	table.sort(sortedDrives,function(a,b)
		return a:value()>b:value()
	end)
	
	for i,drive in pairs(sortedDrives) do
		love.graphics.draw(drive:name() .. ' = ' .. tostring(drive:value()), renderPos[1], renderPos[2])
		renderPos[2] = renderPos[2] + 12
	end
end
