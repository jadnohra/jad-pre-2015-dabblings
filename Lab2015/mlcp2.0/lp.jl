# http://learnxinyminutes.com/docs/julia/
# István Maros. Computational Techniques of the Simplex Method. Kluwer Academic Publishers, Boston, 2003. [CTSM]
# Robert J. Vanderbei. Linear Programming: Foundations and Extensions. Springer, second edition, 2001. [LPFE]

#= 
	Solve, Revised, Simplex. lp_solve_rsimplex
	1. revised simplex, basis inversion, no reuse
	2. revised simplex, basis factorization, no reuse
	3. revised simplex, basis factorization, reuse, no re-factorization
	4. revised simplex, basis factorization, reuse, re-factorization
	5. revised simplex, basis factorization, reuse, re-factorization, degen, sing
	
	Solve, Revised, Sparse. solve_srsimplex
	1. revised sparse simplex

	Presolve
	1. ??
=#

module pretty

	function print(obj)
		#if (typeof(obj) == Int)
	end

end

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

module lp
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


module lp_rsimplex_algo1
	using lp
	using dcd

	type Working_data
		prob::lp.Canonical_problem
		n::Int
		m::Int
		iB::Array{Int, 1}
		iR::Array{Int, 1}
		B#::Array{numtype, 2}
		Binv#::Array{numtype, 2}
		β#::Array{numtype, 1}
		π#::Array{numtype, 1}
		cBT#::Array{numtype, 1}
		dJ#::Array{numtype, 1}
		αq#::Array{numtype, 1} 	
		z#numtype
		zero
		
		# LPFE notation:
		# αq - ΔxB
		# β - xB*
		# q - j
		# i - p
		# t - θ

		Working_data() = new()
	end

	function create_data(prob)
		ret = Working_data()
		ret.prob = prob
		ret.n = prob.n 
		ret.m = prob.m
		ret.iB = Array(Int, prob.m)
		ret.iR = Array(Int, prob.n)
		numtype = prob.numtype; n = prob.n; m = prob.m;
		ret.B = eval(parse( "Array($numtype, ($m,$m))" ))
		#ret.Binv = eval(parse( "Array($numtype, ($m,$m))" ))
		ret.β = eval(parse( "Array($numtype, $m)" ))
		#ret.π = eval(parse( "Array($numtype, $m+$n)" ))
		#ret.cBT = eval(parse( "Array($numtype, $m)" ))
		ret.dJ = eval(parse( "Array($numtype, $n)" ))
		#ret.αq = eval(parse( "Array($numtype, $n)" ))
		ret.zero = eval(parse( "zero($numtype)" ))
		return ret
	end	

	function sel_cols(S, T, cols)
		for i = 1:length(cols)
			c = cols[i]
			for r = 1:size(T)[1]
				T[r,i] = S[r,c]
			end
		end
	end

	function set_basis_logical(data)
		data.iB = [i for i in data.n+1:data.n+data.m]
		data.iR = [i for i in 1:data.n]
	end

	function comp_B_R(data) sel_cols(data.prob.A, data.B, data.iB) end
	function comp_Binv(data) data.Binv = inv(data.B) end
	function comp_cBT(data) data.cBT = transpose(data.prob.c[data.iB]) end
	function init_β(data) data.β = data.Binv * (data.prob.b) end
	function update_β(data, p, θ) data.β = data.β - (θ*data.αq); data.β[p] = θ; end
	function init_z(data) data.z = dot(reshape(data.cBT, length(data.cBT)), data.β); end
	function update_z(data, q, θ) data.z = data.z + θ * data.dJ[q]; end
	function comp_π(data) data.π = reshape(data.cBT * data.Binv, length(data.cBT)); end
	function calc_dj(data, j) i = data.iR[j]; dj = data.prob.c[i] - dot(data.π, data.prob.A[:,i]); return dj; end	
	function comp_dJ(data) for j = 1:data.n data.dJ[j] = calc_dj(data, j) end; end	
	function comp_αq(data, q) aq = data.iR[q]; data.αq = data.Binv * (data.prob.A[:,aq]); end
	function check_optimal_dJ(data) return all( dj->(dj >= data.zero), data.dJ ); end
	function check_feasible_β(data) return all( β->(β >= data.zero), data.β ); end
	
	function pivot_iB_iR(data, q, p) tmp = data.iR[q]; data.iR[q] = data.iB[p]; data.iB[p] = tmp; end 	

	function price_full_dantzig(data)
		# [CTSM].p187
		min_i = 0; min_dj = data.zero;
		for i = 1:length(data.dJ)
			dj = data.dJ[i]
			if (dj < data.zero && dj <= min_dj)
				if (dj == min_dj)
					println("Warning: degeneracy.")
				end	
				min_i = i; min_dj = dj;
			end	
		end
		return min_i
	end	

	function chuzro(data)
		min_i = 0; min_ratio = data.zero;
		for i = 1:length(data.αq)
			if (data.αq[i] > data.zero)
				ratio = data.β[i] / data.αq[i]
				if (min_i == 0 || ratio <= min_ratio)
					if (ratio == min_ratio)
						println("Warning: degeneracy.")
					end	
					min_i = i; min_ratio = ratio;
				end
			end
		end
		return (min_i, min_ratio)
	end

	function succeed_solution(data, sol)
		sol.solved = true 
		sol.status = lp.Optimal
		sol.z = data.z
		sol.x = eval(parse( "zeros($(data.prob.numtype), $(data.n))" ))
		for i = 1:length(data.iB)
			xi = data.iB[i]
			if (xi <= data.n) sol.x[xi] = data.β[i]; end
		end
	end

	function fail_solution(sol, status) sol.solved = false; sol.status = status; end

	function solve_data(data)
		# [CTSM].p33,p30, but with naive basis reinversion instead of update.
		it = 0; maxit = get(data.prob.params, "maxit", 0);
		sol = lp.Solution(get(data.prob.params, "dcd", false)); sol.data = data; sol.prob = data.prob;
		dcd.iter(sol.sess, "init")
		#Step 0
		set_basis_logical(data)
		dcd.set(sol.sess, "iB", data.iB)
		#Initializations
		comp_cBT(data); comp_B_R(data); comp_Binv(data); 
		init_β(data); init_z(data);
		dcd.set(sol.sess, "β0", data.β)
		#todo phaseI
		if (check_feasible_β(data) == false) fail_solution(sol, lp.Infeasible); return sol; end

		while(maxit == 0 || it < maxit)
			if (it != 0) dcd.iter(sol.sess, ""); end
			#Step 1
			dcd.set(sol.sess, "B", data.B); dcd.set(sol.sess, "Binv", data.Binv);
			dcd.set(sol.sess, "cBT", data.cBT);
			comp_π(data); dcd.set(sol.sess, "π", data.π);
			#Step 2
			comp_dJ(data); dcd.set(sol.sess, "dJ", data.dJ);
			if check_optimal_dJ(data) succeed_solution(data, sol); return sol; end
			q = price_full_dantzig(data); dcd.set(sol.sess, "q", q);
			#Step 3
			comp_αq(data, q); dcd.set(sol.sess, "αq", data.αq);
			#Step 4
			p,θ = chuzro(data); dcd.set(sol.sess, "p", p); dcd.set(sol.sess, "θ", θ);
			if (p == 0) fail_solution(sol, lp.Unbounded); return sol; end
			dcd.set(sol.sess, "pivot", (q, p))
			#Step 5
			pivot_iB_iR(data, q, p)
			#Updates
			update_β(data, p, θ); dcd.set(sol.sess, "β", data.β); 
			update_z(data, q, θ); dcd.set(sol.sess, "z", data.z);
			comp_cBT(data); comp_B_R(data); comp_Binv(data); 
			it = it + 1
		end	

		fail_solution(sol, lp.Maxit)
		return sol
	end

	function solve_problem(problem) return solve_data(create_data(problem)) end

end

