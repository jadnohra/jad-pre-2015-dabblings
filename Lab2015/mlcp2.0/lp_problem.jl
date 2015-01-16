
module dcd # Data Crunching Debugger
	type Session
		enable::Bool
		iters::Array{Dict{String,Any}, 1}

		Session(enable = false) = ( x = new(); x.enable = enable; x.iters = Array(Dict{String,Any}, 0); return x; )
	end

	function set(sess, key, val)
		if (sess.enable)
			sess.iters[end][key] = deepcopy(val)
		end	
	end

	function iter(sess, descr)
		if (sess.enable)
			push!(sess.iters, Dict{String,Any}()) 
			set(sess, "descr", descr)
		end	
	end

	function get(sess, it, key)
		return sess.iters[it][key]
	end

	function println(sess, it, key)
		println(get(sess, it, key))
	end
end

module lp_problem
	using dcd

	const Created = "Created"
	const Infeasible = "Infeasible"
	const Optimal = "Optimal"
	const Unbounded = "Unbounded"
	const Maxit = "Maxit"
	const Error = "Error"

	type Canonical_problem #Minimize
		numtype::String
		n::Int
		m::Int
		c#::Array{Any, 1}
		A#::Array{Any, 2}
		b#::Array{Any, 1}
		params::Dict{String,Any}

		Canonical_problem() = new()
	end

	type Solution
		solved::Bool
		status::String
		z#::numtype
		x#::Array{numtype, 1}

		prob::Canonical_problem
		sess::dcd.Session
		data
		
		Solution(enable_dcd = false) = ( x = new(); x.solved = false; x.status = Created; x.sess = dcd.Session(enable_dcd); return x; )
	end	

	function conv_vec(numtype, V)
		n = length(V)
		ret = eval(parse( "Array($numtype, $n)" ))
		for i = 1:n 
			ret[i] = eval(parse("convert($(numtype), $V[$i])"))
		end
		return ret
	end

	function conv_mat(numtype, M)
		r = size(M)[1]; c = size(M)[2];
		ret = eval(parse( "Array($numtype, ($r, $c))" ))
		for i = 1:length(M)
			ret[i] = eval(parse("convert($(numtype), $M[$i])"))
		end
		return ret
	end

	function create_min_problem(numtype, c, A, b, params = {})
		ret = Canonical_problem()
		ret.numtype = numtype
		ret.n = length(c)
		ret.m = length(b)
		ret.c = vcat( conv_vec(numtype, c), eval(parse( "zeros($numtype, $(ret.m))" )) ) 
		ret.A = hcat( conv_mat(numtype, A), conv_mat(numtype, eye(ret.m)) )
		ret.b = conv_vec(numtype, b)
		ret.params = params
		return ret
	end

	function create_max_problem(numtype, c, A, b, params = {}) return create_min_problem(numtype, -1*(c), A, b, params) end
	
	function dcd_var_info(n, i) return i <= n ? ("x", i) : ("w", i-n); end
	function dcd_var(n, i) s,i = dcd_var_info(n, i); return @sprintf "%s%d" s i; end
	function dcd_iR(n, m, iB) 
		iR = Int[] 
		for i = 1:m+n 
			if findfirst(iB, i) == 0 push!(iR, i) end
		end
		return iR
	end			
		
	function dcd_pivots_impl(sol, typ)

		function dcd_print_basis(n, iB)
			for i = 1:length(iB)
				vs,vi = dcd_var_info(n, iB[i])
				@printf "%s%d " vs vi
			end
			@printf "\n"
		end	

		function dcd_print_pivot_type(n, iB, iR, typ)
			if (typ == 2) dcd_print_basis(n, iB)
			elseif (typ == 3) dcd_print_basis(n, iR)
			elseif (typ == 4) println(sort(iB))
			elseif (typ == 5) println(sort(iR))
			end
		end

		function dcd_print_pivot(i, iter, n, iB, iR, typ)
			if (haskey(iter, "pivot"))
				c,r = iter["pivot"]
				if (typ == 1) @printf "%d. %s,%s \n" i dcd_var(n, iR[c]) dcd_var(n, iB[r]); end
				tmp = iR[c]; iR[c] = iB[r]; iB[r] = tmp;
				if (typ != 1) dcd_print_pivot_type(n, iB, iR, typ) end
			end		
		end	

		if isempty(sol.sess.iters) return; end
		n = sol.prob.n
		iB = deepcopy(sol.sess.iters[1]["iB"])
		iR = dcd_iR(sol.prob.n, sol.prob.m, iB)
		if (typ != 1) dcd_print_pivot_type(n, iB, iR, typ) end
		for i = 1:length(sol.sess.iters)
			iter = sol.sess.iters[i]
			dcd_print_pivot(i, iter, n, iB, iR, typ)
		end		
	end

	function dcd_pivots(sol) dcd_pivots_impl(sol, 1) end
	function dcd_basis(sol) dcd_pivots_impl(sol, 2) end
	function dcd_nbasis(sol) dcd_pivots_impl(sol, 3) end
	function dcd_ibasis(sol) dcd_pivots_impl(sol, 4) end
	function dcd_inbasis(sol) dcd_pivots_impl(sol, 5) end
	function dcd_iters(sol) println(length(sol.sess.iters)) end
	function dcd_key(sol, key)	
		for i = 1:length(sol.sess.iters)
			iter = sol.sess.iters[i]
			if (haskey(iter, key))
				val = iter[key]
				println("$i. $val")
			end
		end	
	end
end

