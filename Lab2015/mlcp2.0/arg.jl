
module arg

	export arg_create
	export arg_key
	export arg_has
	export arg_get

	type ArgPart
		start::Int
		sz::Int
		pid::Int

		ArgPart() = ( x = new(); x.start = 1; x.sz = 0; x.pid = -1; return x; )
	end

	type ArgKey
		keys::Vector{String}
		iter::Vector{Int}
		match::Vector{Int}

		ArgKey() = new()
	end

	arg_key() = ( x = ArgKey(); x.keys = Array(String, 0); x.iter = Array(Int, 0); x.match = Array(Int, 0); return x; )
	arg_key(k::String) = ( x = arg_key(); add_key(x, k); return x; )
	arg_key(prev_ak::ArgKey, k::String) = ( x = arg_key(); add_key(x, prev_ak, k); return x; )

	function add_key(ak::ArgKey, k::String, it = 0, mtch = -1)
		push!(ak.keys, k); push!(ak.iter, it); push!(ak.match, mtch);
	end

	function add_key(ak::ArgKey, prev_ak::ArgKey, k::String)
		for i=1:length(prev_ak.keys)
			add_key(ak, prev_ak.keys[i]. prev_ak.iter[i], prev_ak.match[i])
		end
		add_key(ak, k)
	end

	function key_length(ak::ArgKey)
		return length(ak.keys)
	end

	type Node
		pid::Int
		id::Int
		key_start::Int
		key_sz::Int
		key_first::Char
		val_is_node::Bool
		val_start::Int
		val_sz::Int
		depth::Int
	end

	type Args
		str::Vector{Char}
		len::Int
		str_orig::String
		nodes::Vector{Node}

		Args() = ( x = new(); x.str = []; x.str_orig = ""; x.len = 0; x.nodes = Node[]; return x; )
	end

	function _parse_recurse(args, depth::Int, pid::Int, str_i::Int, next_str_i::Array{Int, 1})
		const char0 = char(0)
		str = args.str
		while(isempty(str) == false && str[str_i] != char0)
			if (str[str_i] == '/')
				str[str_i] = char0
				id = length(args.nodes)
				key_start = str_i+1; i = key_start;
				while (str[i] != char0 && str[i] != ':' && str[i] != '/' && str[i] != '}') i = i+1; end;
				node; success = false;
				if (str[i] == char0 || str[i] == '/' || str[i] == '}')
					node2 = Node(pid, id, key_start, i-key_start, str[key_start], false, -1, 0, depth); node = node2; success = true;
					str_i = i
				elseif (str[i] == ':')
					str[i] = char0
					if (str[i+1] != '{')
						val_start = i+1
						vi = val_start; while (str[vi] != char0 && str[vi] != '/' && str[vi] != '}') vi = vi+1; end;
						node2 = Node(pid, id, key_start, i-key_start, str[key_start], false, val_start, vi-val_start, depth); node = node2; success = true;
						str_i = vi
					elseif (str[i+1] == '{')
						cont_str_i = [0]
						val_start = length(args.nodes); if (!_parse_recurse(args, depth+1, id, i+2, cont_str_i)) return false; end; val_sz = length(args.nodes)-val_start;
						node2 = Node(pid, id, key_start, i-key_start, str[key_start], args.nodes, val_start, val_sz, depth); node = node2; success = true;
						str_i = cont_str_i[0]
					end
				end

				if (success)
					push!(args.nodes, node)
				end

				if (str[str_i] == '}')
					str[str_i] = char0
					if (length(next_str_i) > 0 ) next_str_i[1] = str_i + 1; end
					return true
				end
			elseif (length(next_str_i) > 0 && str[str_i] == '}')
				str[str_i] = char0
				next_str_i[1] = str_i + 1
				return true
			else
				return false
			end
		end
		return true
	end

	function _key_str(str, node::Node)
		return join(str[node.key_start:node.key_start+node.key_sz-1])
	end

	function _matches(str, node::Node, key, depth::Int)
		if (node.depth == depth && node.key_first == key[1] && node.key_sz == length(key))
			for j = 1:node.key_sz
			 	if (key[j] != str[node.key_start+j-1]) return false; end;
			end
			return true
		end
		return false
	end

	function _get_empty_sub()
		return ArgPart()
	end

	function _get_root_sub(args::Args)
		ret = ArgPart(); ret.start = 1; ret.sz = length(args.nodes); ret.pid = -1; return ret;
	end

	function _to_part(args::Args, ni::Int)
		ret = _get_empty_sub()
		if (ni >= 1 && ni <= length(args.nodes))
			ret.start = args.nodes[ni].val_start; ret.sz = args.nodes[ni].val_sz; ret.pid = args.nodes[ni].id;
		end
		return ret
	end

	function _find_recurse(args::Args, sub::ArgPart, key::ArgKey, ki::Int)
		if (ki > key_length(key)) return -1; end;
		if (key.match[ki] != -1)
			found_ni = key.match[ki]
			if (ki == key_length(key)) return found_ni; end;
			return _find_recurse(args, _to_part(args, found_ni), key, ki+1)
		end
		matchi = 0;
		for ni = sub.start:(sub.start+sub.sz-1)
			found_ni = -1
			if (_matches(args.str, args.nodes[ni], key.keys[ki], ki))
				if (matchi == key.iter[ki])
					if (ki == key_length(key))
						found_ni = ni
					else
						if (args.nodes[ni].val_is_node)
							found_ni = _find_recurse(args, _to_part(args, ni), key, ki+1)
						end
					end
				else
					found_ni = -1
					matchi = matchi+1
				end
			end
			if (found_ni != -1) key.match[ki] = found_ni; return found_ni; end;
		end
		return -1
	end

	function _find(args::Args, key::ArgKey)
		return _find_recurse(args, _get_root_sub(args), key, 1)
	end

	function _iter_key(args::Args, index::Int, key::ArgKey)
		found_ni = _find(args, key)
		if (found_ni < 0 || args.nodes[found_ni].val_is_node == false || args.nodes[found_ni].val_sz <= index) return -1; end;
		return args.nodes[found_ni].val_start + index
	end

	function arg_create(str::String)
		args = Args()
		args.str_orig = str
		args.str = Array(Char, length(str)+1); args.str[end] = char(0);
		for i = 1:length(str) args.str[i] = str[i]; end
		_parse_recurse(args, 1, -1, 1, Array(Int, 0))
		return args
	end

	function root(args::Args)
		return _get_root_sub(args)
	end

	function to_key(k::Any)
		return typeof(k) == ArgKey ? k : arg_key(k)
	end

	function get_part(args::Args, key)
		key = to_key(key)
		ni = _find(args, key); val = _to_part(args, ni);
		return ((ni != -1 && !args.nodes[ni].val_is_node), val)
	end

	function arg_has(args::Args, key)
		key = to_key(key)
		return get_part(args, key)[1]
	end

	function has_sub(args::Args, key)
		key = to_key(key)
		ni = _find(args, key);
		return (ni != -1 && args.nodes[ni].val_is_node)
	end

	function iter(args::Args, key)
		key = to_key(key)
		if (key.match[key_length(key)] == -1) return has_sub(key); end;
		if (key_length(key)) key.iter[end] = key.iter[end]+1; key.match[end] = -1; end;
		return has_sub(args, key)
	end

	function parse_s(args::Args, key, dflt::String)
		key = to_key(key)
		found, val = get_part(args, key)
		if (found == false) return dflt; end;
		return join(args.str[val.start:val.start+val.sz-1])
	end

	function arg_get(args::Args, key, dflt::Any)
		key = to_key(key)
		typ = string(typeof(dflt))
		str = parse_s(args, key, "")
		if (length(str) == 0 || typeof(dflt) == String) return dflt; end;
		ret = dflt
		try
			ret = eval(Base.parse("convert($(typ), \"$str\")"))
		catch
			ret = eval(Base.parse("convert($(typ), $str)"))
		end
		return ret
	end

	function arg_get(args::Args, keys::Dict{Any, Any})
		for k in keys
			keys[k[1]] = arg_get(args, k[1], k[2])
		end

		for i = 1:length(args.nodes)
			node = args.nodes[i]
			if (node.val_is_node == false)
				key = _key_str(args.str, node)
				if (haskey(keys, key) == false)
					keys[key] = parse_s(args, key, "")
				end
			end
		end
	end

	#TODO. convert the rest of the c++ code.

end
