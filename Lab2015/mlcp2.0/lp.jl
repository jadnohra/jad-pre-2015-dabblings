

module lp
	using dcd

	:Created
	:Infeasible
	:Optimal
	:Unbounded
	:Maxit
	:Error

	typealias Params Dict{Any,Any}

	type Index_translation
		index::Int
	end

	type SingleVar_translation{T}
		index::Int
		op1_mul::T
		op2_add::T
		SingleVar_translation() = (x = new(); index = -1; op1_mul = one(T); op2_add = zero(T); return x;)
	end

	type Recover_translation{T}
		a::Vector{T}
		b::T
		var_index::Int
		Recover_translation() = new()
	end

	type Canonical_translation{T}
		z_transl::SingleVar_translation{T}
		x_transl::Vector{Any}

		function Canonical_translation()
			x = new()
			x.z_transl = SingleVar_translation{T}()
			x.x_transl = Array(Any, 0)
			return x
		end
	end

	type Canonical_problem{T} #Minimize
		numtype::String
		n::Int
		m::Int
		c::Vector{T}
		A::Matrix{T}
		b::Vector{T}
		params::Params
		transl::Canonical_translation{T}

		Canonical_problem() = new()
	end

	type Solution{T}
		solved::Bool
		status::Symbol
		z::T
		x::Vector{T}

		dcd::dcd.Session

		function Solution(params::Params)
			x = new()
			x.solved = false
			x.status = :Created
			x.dcd = dcd.Session(get(params, "dcd", false))
			return x
		end
	end

	function conv(numtype::String, v)
		return eval(parse("convert($(numtype), $v)"))
	end

	function conv(numtype::String, V::Vector)
		n = length(V)
		ret = eval(parse( "Array($numtype, $n)" ))
		for i = 1:n
			ret[i] = conv(numtype, V[i])
		end
		return ret
	end

	function conv(numtype::String, M::Matrix)
		r = size(M)[1]; c = size(M)[2];
		ret = eval(parse( "Array($numtype, ($r, $c))" ))
		for i = 1:length(M)
			ret[i] = eval(parse("convert($(numtype), $M[$i])"))
		end
		return ret
	end

	function comp_density{T}(prob::Canonical_problem{T})
		A = prob.A[1:prob.m,1:prob.n]
		nz = 0
		for a in A
			nz = nz + ((a == zero(T)) ? 1 : 0)
		end
		return one(T) - (convert(T, nz) / convert(T, prob.m*prob.n))
	end

	function create_solution(numtype::String, params::Params)
		expr_create = parse( "Solution{$numtype}($params)" )
		ret = eval(expr_create)
		return ret
	end

	function transl_single{T}(transl::SingleVar_translation{T}, v::T)
		return (v*transl.op1_mul)+transl.op2_add
	end

	function transl_recover{T}(prob, transl::Recover_translation, v::T)
		return v
	end

	function transl_any{T}(prob::Canonical_problem{T}, transl::Any, x::Vector{T})
		if (typeof(transl) == Index_translation)
			return x[transl.index]
		elseif (typeof(transl) == SingleVar_translation)
			return transl_single(transl, x[transl.index])
		elseif (typeof(transl) == Recover_translation)
			return transl_recover(prob, transl, x[transl.index])
		end
	end

	function translate_solution{T}(prob::Canonical_problem{T}, can_sol::Solution{T})
		transl = prob.transl

		gen_sol =create_solution(prob.numtype, prob.params)
		gen_sol.solved = can_sol.solved
		gen_sol.status = can_sol.status
		gen_sol.z = transl_single(transl.z_transl, can_sol.z)
		gen_sol.dcd = can_sol.dcd

		if (can_sol.solved)
			if (length(transl.x_transl) > 0)
				x_n = length(transl.x_transl)
				gen_sol.x = Array(T, x_n)
				for i = 1:x_n
					gen_sol.x[i] = transl_any(prob, transl.x_transl[i], can_sol.x)
				end
			else
				gen_sol.x = can_sol.x
			end
		end

		return gen_sol
	end

	# min: z = cx, subj: Ax <= b, x >= 0
	function create_min_canonical_problem(params::Params, c::Vector, A::Matrix, b::Vector)
		numtype = params["type"]
		expr_create = parse( "Canonical_problem{$numtype}()" )
		ret = eval(expr_create)
		ret.numtype = numtype
		ret.n = length(c)
		ret.m = length(b)
		ret.c = vcat( conv(numtype, c), eval(parse( "zeros($numtype, $(ret.m))" )) )
		ret.A = hcat( conv(numtype, A), conv(numtype, eye(ret.m)) )
		ret.b = conv(numtype, b)
		ret.params = params
		ret.transl = eval( parse( "Canonical_translation{$numtype}()" ) )
		return ret
	end

	# max: z = cx, subj: Ax <= b, x >= 0
	function create_max_canonical_problem(params::Params, c::Vector, A::Matrix, b::Vector)
		numtype = params["type"]
		mone = conv(numtype, -1)
		ret = create_min_canonical_problem(params, mone*(c), A, b)
		ret.transl.z_transl.op1_mul = mone
		return ret
	end

	#=
	# min: z = cx, subj: a <= Ax <= b, xlo <= x <= xhi
	function create_min_gen_problem(params::Params, c::Vector, A::Matrix, a::Vector, b::Vector, xlo::Vector, xhi::Vector)
		numtype = params["type"]
		transl = eval( parse( "Canonical_translation{$numtype}()" ) )
		n = length(c)
		mone = conv(numtype, -1)
		one = conv(numtype, 1)

		# translate a,b bounds
		# todo: detect conflicts, or constraints that only bound a single variable, etc.
		if (length(a) != length(b)) return 0; end
		if (length(a) != size(A)[1]) return 0; end
		if (length(c) != size(A)[2]) return 0; end
		A1 = Array(Array{Any} , 0)
		b1 = Array(Any, 0)
		for i=1:length(a)
			if (a[i] > b[i])
				return 0
			elseif (a[i] == -Inf && b[i] == Inf)
				# not constraining
			else
				if (b[i] != Inf)
					push!(A1, deepcopy(A[i,:])); push!(b1, deepcopy(b[i]))
				end
				if (a[i] != -Inf)
					push!(A1, mone * deepcopy(A[i,:])); push!(b1, mone * deepcopy(a[i]))
				else
			end
		end

		# translate xlo, xhi bounds
		Aex = Array(Array{Any} , 0)
		bex = Array(Any, 0)
		cex = Array(Any, 0)
		if (length(c) != length(xlo)) return 0; end
		if (length(xhi) != length(xlo)) return 0; end
		transl.x_transl = Array(Any, length(c))
		for i=1:length(c)
			xi_transl = Index_translation(i)
			if (xlo[i] > xhi[i])
				return 0
			elseif (xlo[i] == -Inf && xhi[i] == Inf)
				# free, eliminate
				# xi_transl = Recover_translation()
				# todo, maybe as a separate pass, after this one.
				return 0
			else
				if (xhi[i] == Inf)
					xi_transl = SingleVar_translation()
					xi_transl.index = i
					xi_transl.op1_mul = conv(one)
					xi_transl.op2_add = conv(xlo[i])
				elseif (xlo[i] == -Inf)
					xi_transl = SingleVar_translation()
					xi_transl.index = i
					xi_transl.op1_mul = conv(mone)
					xi_transl.op2_add = conv(xhi[i])
				else
					xi_transl = SingleVar_translation()
					xi_transl.index = i
					xi_transl.op1_mul = conv(one)
					xi_transl.op2_add = conv(xlo[i])
					Aex
				end
			end
			transl.x_transl[i] = xi_transl
	end
	=#

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

		if isempty(sol.dcd.iters) return; end
		n = sol.prob.n
		iB = deepcopy(sol.dcd.iters[1]["iB"])
		iR = dcd_iR(sol.prob.n, sol.prob.m, iB)
		if (typ != 1) dcd_print_pivot_type(n, iB, iR, typ) end
		for i = 1:length(sol.dcd.iters)
			iter = sol.dcd.iters[i]
			dcd_print_pivot(i, iter, n, iB, iR, typ)
		end
	end

	function dcd_pivots(sol) dcd_pivots_impl(sol, 1) end
	function dcd_basis(sol) dcd_pivots_impl(sol, 2) end
	function dcd_nbasis(sol) dcd_pivots_impl(sol, 3) end
	function dcd_ibasis(sol) dcd_pivots_impl(sol, 4) end
	function dcd_inbasis(sol) dcd_pivots_impl(sol, 5) end
	function dcd_iters(sol) println(length(sol.dcd.iters)) end
	function dcd_key(sol, key)
		for i = 1:length(sol.dcd.iters)
			iter = sol.dcd.iters[i]
			if (haskey(iter, key))
				val = iter[key]
				println("$i. $val")
			end
		end
	end
end
