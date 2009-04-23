
BoxEntity = class(EntityBase, function(a, name, x, y, w, h)
   EntityBase.init(a, name) 
   a.box = Box2D(x,y,w,h)
end)

function BoxEntity:getPos()
	return self.box.pos
end

function BoxEntity:contains(point)
	return self.box:contains(point)
end

function BoxEntity:moveTo(point)
	self.box.pos = point
end

function BoxEntity:draw(engine, camera)
   
   local renderPos = camera:transform(self.box.pos)
   
   love.graphics.setColor(self.color)
	
   love.graphics.draw(self:description(), renderPos[1], renderPos[2])
   
   local halfw = 0.5 * self.box.size[1] * camera.scale
   local halfh = 0.5 * self.box.size[2] * camera.scale
   
   love.graphics.rectangle(love.draw_line, renderPos[1] - halfw, renderPos[2] - halfh, halfw + halfw, halfh + halfh)
end


