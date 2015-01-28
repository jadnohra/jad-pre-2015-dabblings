
module Conv

function conv_func(t::DataType)
	return (x -> convert(t, x))
end

function conv_func(typestr::String)
	return eval(parse(" x -> convert($(typestr), x)"))
end

function conv(t::DataType, v::Any)
	return convert(t, v)
end

function conv(typestr::String, v::Any)
	return conv_func(typestr)(v)
end

function conv_type(typestr::String)
	return eval(parse("$typestr"))
end

type Converter
	f::Function
	t::DataType
end

function converter(t::DataType)
	return Converter(conv_func(t), t)
end

function converter(typestr::String)
	return converter(conv_type(typestr))
end

function vector(conv::Converter, V::Vector)
	n = length(V)
	ret = Array(conv.t, n)
	for i = 1:n
		ret[i] = conv.f(V[i])
	end
	return ret
end

function matrix(conv::Converter, M::Matrix)
	r = size(M)[1]; c = size(M)[2];
	ret = Array(conv.t, (r, c))
	for i = 1:length(M)
		ret[i] = conv.f(M[i])
	end
	return ret
end


end
