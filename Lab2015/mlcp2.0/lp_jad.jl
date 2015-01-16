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
	7. 5. with Maros' Phase-I (p203)
	
	II. Solve, Revised, Sparse, Canonical Simplex. solve_srsimplex
	1. revised sparse simplex

	III. Generalized Simplex variants of I, then II (Presolve, Postolve)
	1. ??
=#

module lp_jad_I_1
	using dcd
	using lp

	type Dat{T}
		prob::lp.Canonical_problem
		maxit::Int
		n::Int
		m::Int
		# Using CTSM notation.
		iB::Array{Int, 1}
		iR::Array{Int, 1}
		B::Array{T, 2}
		Binv::Array{T, 2}
		β::Array{T, 1}
		π::Array{T, 1}
		cBT::Array{T, 2}
		dJ::Array{T, 1}
		αq::Array{T, 1} 	
		z::T
		zero::T
		
		# LPFE notation translation.
		# αq - ΔxB, β - xB*, q - j, i - p, t - θ

		Dat() = new()
	end

	function fill_dat{T}(prob::lp.Canonical_problem, dat::Dat{T})
		n = prob.n; m = prob.m;
		dat.maxit = get(prob.params, "maxit", 0)
		dat.prob = prob
		dat.n = n 
		dat.m = m
		dat.iB = Array(Int, m)
		dat.iR = Array(Int, n)
		dat.B = Array(T, (m,m)) 
		#dat.Binv = eval(parse( "Array($numtype, ($m,$m))" ))
		dat.β = Array(T, m) 
		#dat.π = eval(parse( "Array($numtype, $m+$n)" ))
		#dat.cBT = eval(parse( "Array($numtype, $m)" ))
		dat.dJ = Array(T, n) 
		#dat.αq = eval(parse( "Array($numtype, $n)" ))
		dat.zero = zero(T) 
	end	

	function sel_cols{T}(src::Matrix{T}, dst::Matrix{T}, cols::Vector{Int})
		for i = 1:length(cols)
			c = cols[i]
			for r = 1:size(dst)[1]
				dst[r,i] = src[r,c]
			end
		end
	end

	function set_basis_logical{T}(dat::Dat{T})
		dat.iB = [i for i in dat.n+1:dat.n+dat.m]
		dat.iR = [i for i in 1:dat.n]
	end

	function comp_B_R{T}(dat::Dat{T}) sel_cols(dat.prob.A, dat.B, dat.iB) end
	function comp_Binv{T}(dat::Dat{T}) dat.Binv = inv(dat.B) end
	function comp_cBT{T}(dat::Dat{T}) dat.cBT = transpose(dat.prob.c[dat.iB]) end
	function init_β{T}(dat::Dat{T}) dat.β = dat.Binv * (dat.prob.b) end
	function update_β{T}(dat::Dat{T}, p::Int, θ::T) dat.β = dat.β - (θ*dat.αq); dat.β[p] = θ; end
	function init_z{T}(dat::Dat{T}) dat.z = dot(reshape(dat.cBT, length(dat.cBT)), dat.β); end
	function update_z{T}(dat::Dat{T}, q::Int, θ::T) dat.z = dat.z + θ * dat.dJ[q]; end
	function comp_π{T}(dat::Dat{T}) dat.π = reshape(dat.cBT * dat.Binv, length(dat.cBT)); end
	function calc_dj{T}(dat::Dat{T}, j::Int) i = dat.iR[j]; dj = dat.prob.c[i] - dot(dat.π, dat.prob.A[:,i]); return dj; end	
	function comp_dJ{T}(dat::Dat{T}) for j = 1:dat.n dat.dJ[j] = calc_dj(dat, j) end; end	
	function comp_αq{T}(dat::Dat{T}, q::Int) aq = dat.iR[q]; dat.αq = dat.Binv * (dat.prob.A[:,aq]); end
	function check_optimal_dJ{T}(dat::Dat{T}) return all( dj->(dj >= dat.zero), dat.dJ ); end
	function check_feasible_β{T}(dat::Dat{T}) return all( β->(β >= dat.zero), dat.β ); end
	
	function pivot_iB_iR{T}(dat::Dat{T}, q::Int, p::Int) tmp = dat.iR[q]; dat.iR[q] = dat.iB[p]; dat.iB[p] = tmp; end 	

	function price_full_dantzig{T}(dat::Dat{T})
		# [CTSM].p187
		min_i = 0; min_dj = dat.zero;
		for i = 1:length(dat.dJ)
			dj = dat.dJ[i]
			if (dj < dat.zero && dj <= min_dj)
				if (dj == min_dj)
					println("Warning: degeneracy.")
				end	
				min_i = i; min_dj = dj;
			end	
		end
		return min_i
	end	

	function chuzro{T}(dat::Dat{T})
		min_i = 0; min_ratio = dat.zero;
		for i = 1:length(dat.αq)
			if (dat.αq[i] > dat.zero)
				ratio = dat.β[i] / dat.αq[i]
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

	function succeed_solution{T}(dat::Dat{T}, sol::lp.Solution)
		sol.solved = true 
		sol.status = :Optimal
		sol.z = dat.z
		sol.x = eval(parse( "zeros($(dat.prob.numtype), $(dat.n))" ))
		for i = 1:length(dat.iB)
			xi = dat.iB[i]
			if (xi <= dat.n) sol.x[xi] = dat.β[i]; end
		end
	end

	function fail_solution(sol::lp.Solution, status::Symbol) sol.solved = false; sol.status = status; end

	function solve_dat{T}(dat::Dat{T}, sol::lp.Solution)
		# [CTSM].p33,p30, but with naive basis reinversion instead of update.
		dbg = sol.dcd
		dcd.@it(dbg)
		it = 0
		#Step 0
		set_basis_logical(dat)
		dcd.@set(dbg, "iB", dat.iB)
		#Initializations
		comp_cBT(dat); comp_B_R(dat); comp_Binv(dat); 
		init_β(dat); init_z(dat);
		dcd.@set(dbg, "β0", dat.β) 
		#todo phaseI
		if (check_feasible_β(dat) == false) println("Warning: phaseI."); fail_solution(sol, :Infeasible); return; end

		while(dat.maxit == 0 || it < dat.maxit)
			dcd.@it(dbg)
			#Step 1
			dcd.@set(dbg, "B", dat.B); dcd.@set(dbg, "Binv", dat.Binv); 
			dcd.@set(dbg, "cBT", dat.cBT); 
			comp_π(dat); dcd.@set(dbg, "π", dat.π); 
			#Step 2
			comp_dJ(dat); dcd.@set(dbg, "dJ", dat.dJ); 
			if check_optimal_dJ(dat) succeed_solution(dat, sol); return; end
			q = price_full_dantzig(dat); dcd.@set(dbg, "q", q); 
			#Step 3
			comp_αq(dat, q); dcd.@set(dbg, "αq", dat.αq); 
			#Step 4
			p,θ = chuzro(dat); dcd.@set(dbg, "p", p); dcd.@set(dbg, "θ", θ); 
			if (p == 0) fail_solution(sol, :Unbounded); return; end
			dcd.@set(dbg, "pivot", (q, p)) 
			#Step 5
			pivot_iB_iR(dat, q, p)
			#Updates
			update_β(dat, p, θ); dcd.@set(dbg, "β", dat.β); 
			update_z(dat, q, θ); dcd.@set(dbg, "z", dat.z); 
			comp_cBT(dat); comp_B_R(dat); comp_Binv(dat); 
			it = it + 1
		end	
		fail_solution(sol, :Maxit)
	end

	function solve_problem(lp_prob) 
		expr_Dat = parse( "Dat{$(lp_prob.numtype)}()" )
		dat = eval(expr_Dat)
		fill_dat(lp_prob, dat)
		sol = lp.create_solution(lp_prob.numtype, lp_prob.params)
		solve_dat(dat, sol) 
		return sol
	end

end

