
Circle2D = class(function(a, x, y, r)
   a.pos = Vector2D(x, y)
   a.radius = r
end)

function Circle2D:contains(vector2D)
	return vector2D:dist(self.pos) <= self.radius
end	
