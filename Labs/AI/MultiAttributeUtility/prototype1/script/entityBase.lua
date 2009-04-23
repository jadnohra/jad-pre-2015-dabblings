
EntityBase = class(UIWidget, function(a, name)
   UIWidget.init(a) 
   a.name = name
   a.color = nil
   a.drives = nil
end)

--function EntityBase:contains(point)
--	return point:dist(self.pos) <= self.radius
--end

--function EntityBase:moveTo(point)
--	self.pos = point
--end

function EntityBase:description()
	return self.name
end

function EntityBase:draw(engine, camera)
   
   --local renderPos = camera:transform(self.pos)
   
   --love.graphics.setColor(self.color)
	
   --love.graphics.draw(self:description(), renderPos[1], renderPos[2])
   --love.graphics.circle(love.draw_line, renderPos[1], renderPos[2], self.radius * camera.scale)
end


