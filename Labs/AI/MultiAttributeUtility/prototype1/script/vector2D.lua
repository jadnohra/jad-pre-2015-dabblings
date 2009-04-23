
Vector2D = class(function(a, x, y)
   a[1] = x
   a[2]= y
end)

function Vector2D:dist(vector2D)
	
	local xDist = self[1] - vector2D[1]
	local yDist = self[2] - vector2D[2]
		
	return math.sqrt(xDist * xDist + yDist * yDist)
end	
