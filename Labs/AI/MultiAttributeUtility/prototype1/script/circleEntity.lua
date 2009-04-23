
CircleEntity = class(EntityBase, function(a, name, x, y, r)
   EntityBase.init(a, name) 
   a.circle = Circle2D(x,y,r)
end)

function CircleEntity:getPos()
	return self.circle.pos
end

function CircleEntity:contains(point)
	return self.circle:contains(point)
end

function CircleEntity:moveTo(point)
	self.circle.pos = point
end

function CircleEntity:draw(engine, camera)
   
   local renderPos = camera:transform(self.circle.pos)
   
   love.graphics.setColor(self.color)
	
   love.graphics.draw(self:description(), renderPos[1], renderPos[2])
   love.graphics.circle(love.draw_line, renderPos[1], renderPos[2], self.circle.radius * camera.scale)
end


