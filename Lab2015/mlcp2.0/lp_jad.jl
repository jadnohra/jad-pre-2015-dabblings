# István Maros. Computational Techniques of the Simplex Method. Kluwer Academic Publishers, Boston, 2003. [CTSM]
# Robert J. Vanderbei. Linear Programming: Foundations and Extensions. Springer, second edition, 2001. [LPFE]

#= 
	I. Solve, Revised, Canonical Simplex. 
	1. revised simplex, basis inversion, no reuse
	2. revised simplex, basis factorization, no reuse
	3. revised simplex, basis factorization, reuse, no re-factorization
	4. revised simplex, basis factorization, reuse, re-factorization
	5. revised simplex, basis factorization, reuse, re-factorization, degen (CTSM p230), sing
	6. 5. with 'textbook' Phase-I
	7. 5. with Maros Phase-I ()
	
	II. Solve, Revised, Sparse, Canonical Simplex. solve_srsimplex
	1. revised sparse simplex

	III. Generalized Simplex variants of I, then II (Presolve, Postolve)
	1. ??
=#

module lp_jad_I_1
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
		if (check_feasible_β(data) == false) println("Warning: phaseI."); fail_solution(sol, lp.Infeasible); return sol; end

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

