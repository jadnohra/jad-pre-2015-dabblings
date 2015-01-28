# István Maros. Computational Techniques of the Simplex Method. Kluwer Academic Publishers, Boston, 2003. [CTSM]
# Robert J. Vanderbei. Linear Programming: Foundations and Extensions. Springer, second edition, 2001. [LPFE]

#=
	List of algorithms

	Linear Programming
		I. Solve, Revised, Canonical Simplex.
			1. revised simplex, basis inversion, no reuse
			2. revised simplex, basis factorization, no reuse
			3. revised simplex, basis factorization, reuse, no re-factorization
			4. revised simplex, basis factorization, reuse, re-factorization
			5. revised simplex, basis factorization, reuse, re-factorization, degen (CTSM p230), sing
			6. 5. with 'textbook' Phase-I
			7. 5. with Maros' Phase-I (p203)

		II. Solve, Revised, Sparse, Canonical Simplex.
			1. revised sparse simplex

		III. Generalized Simplex variants of I, then II (Presolve, Postsolve)
			1. generalized version of I.1, using Maros' CF-2 (p13-17), but without translation, calling it CF-2b
			2. 1. with variable transformation for less lohi calculations, the proper CF-2 formulation
			3. 2. with infeasibility tolerance, type-0 handling and Harris ratio. (CSTM p178)

	LU Factorization
		I. Suhl-suhl (CTSM p136)
=#

module Jad

module lp_I_1
	using Dcd
	using Lp

	type Dat{T}
		prob::Lp.Cf0_problem
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

	function construct_dat(T::DataType) return Dat{T}() end

	function fill_dat{T}(prob::Lp.Cf0_problem, dat::Dat{T})
		n = prob.n; m = prob.m;
		dat.maxit = get(prob.params, "maxit", 0)
		dat.prob = prob
		dat.n = n
		dat.m = m
		dat.iB = Array(Int, m)
		dat.iR = Array(Int, n)
		dat.B = Array(T, (m,m))
		#dat.Binv = Array(T, (m,m))
		dat.β = Array(T, m)
		#dat.π = Array(T, m+n)
		#dat.cBT = Array(T, m)
		dat.dJ = Array(T, n)
		#dat.αq = Array(T, n)
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

	function pivot_iB_iR{T}(dat::Dat{T}, q::Int, p::Int) dat.iR[q], dat.iB[p] = dat.iB[p], dat.iR[q] end

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
			if (dat.αq[i] != dat.zero)
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

	function succeed_solution{T}(it::Int, dat::Dat{T}, sol::Lp.Solution)
		sol.iter = it
		sol.solved = true
		sol.status = :Optimal
		sol.z = dat.z
		sol.x = zeros(dat.prob.conv.t, dat.n)
		for i = 1:length(dat.iB)
			xi = dat.iB[i]
			if (xi <= dat.n) sol.x[xi] = dat.β[i]; end
		end
	end

	function fail_solution(it::Int, sol::Lp.Solution, status::Symbol) sol.iter = it, sol.solved = false; sol.status = status; end

	function solve_dat{T}(dat::Dat{T}, sol::Lp.Solution{T})
		# [CTSM].p33,p30, but with naive basis reinversion instead of update.
		dbg = sol.Dcd
		Dcd.@it(dbg)
		it = 0
		#Step 0
		set_basis_logical(dat)
		Dcd.@set(dbg, "iB", dat.iB)
		#Initializations
		comp_cBT(dat); comp_B_R(dat); comp_Binv(dat);
		init_β(dat); init_z(dat);
		Dcd.@set(dbg, "β0", dat.β)
		#todo phaseI
		if (check_feasible_β(dat) == false) println("Warning: phaseI."); fail_solution(it, sol, :Infeasible); return; end

		while(dat.maxit == 0 || it < dat.maxit)
			Dcd.@it(dbg)
			#Step 1
			Dcd.@set(dbg, "B", dat.B); Dcd.@set(dbg, "Binv", dat.Binv);
			Dcd.@set(dbg, "cBT", dat.cBT);
			comp_π(dat); Dcd.@set(dbg, "π", dat.π);
			#Step 2
			comp_dJ(dat); Dcd.@set(dbg, "dJ", dat.dJ);
			if check_optimal_dJ(dat) succeed_solution(it, dat, sol); return; end
			q = price_full_dantzig(dat); Dcd.@set(dbg, "q", q);
			#Step 3
			comp_αq(dat, q); Dcd.@set(dbg, "αq", dat.αq);
			#Step 4
			p,θ = chuzro(dat); Dcd.@set(dbg, "p", p); Dcd.@set(dbg, "θ", θ);
			if (p == 0) fail_solution(it, sol, :Unbounded); return; end
			Dcd.@set(dbg, "pivot", (q, p))
			#Step 5
			pivot_iB_iR(dat, q, p)
			#Updates
			update_β(dat, p, θ); Dcd.@set(dbg, "β", dat.β);
			update_z(dat, q, θ); Dcd.@set(dbg, "z", dat.z);
			comp_cBT(dat); comp_B_R(dat); comp_Binv(dat);
			it = it + 1
		end
		fail_solution(it, sol, :Maxit)
	end

end

module lp_III_1
	using Dcd
	using Lp
	using Jad.lp_I_1

	type Dat{T}
		cf0::lp_I_1.Dat{T}
		prob::Lp.Cf2b_problem{T}

		Dat() = new()
	end

	function construct_dat(T::DataType) return Dat{T}() end

	function fill_dat{T}(prob::Lp.Cf2b_problem{T}, dat::Dat{T})
		Base = lp_I_1
		dat.cf0 = Base.construct_dat(T)
		Base.fill_dat(prob.cf0, dat.cf0)
		dat.prob = prob
	end

	function get_lohi{T}(dat::Dat{T}, i::Int)
		bi = 2*dat.iB[i]
		return dat.prob.lohi[bi], dat.prob.lohi[bi+1]
	end

	function warn{T}(dat::Dat{T}, warning::Symbol)
		println("Warning:", warning)
	end

	function chuzro{T}(dat::Dat{T})
		# Handwritten notes (p29)
		i_θhi = 0; min_θhi = dat.zero;
		i_θlo = 0; max_θlo = dat.zero;

		for i = 1:length(dat.αq)
			if (dat.αq[i] > dat.zero)
				lo, hi = get_lohi(dat, i)
				θhi, θlo = (dat.β[i] - lo) / dat.αq[i], (dat.β[i] - hi) / dat.αq[i]

				if (i_θlo == 0 || θlo >= max_θlo)
					if (θlo == max_θlo)
						warn(dat, :Degen)
					end
					i_θlo = i; max_θlo = θlo;
				end

				if (i_θhi == 0 || θhi <= min_θhi)
					if (θhi == min_θhi)
						warn(dat, :Degen)
					end
					i_θhi = i; min_θhi = θhi;
				end

			end
		end

		return (min_i, min_ratio)
	end

	function solve_dat{T}(dat2b::Dat{T}, sol::Lp.Solution{T})
		# [CTSM].p33,p30, but with naive basis reinversion instead of update.
		# Additionally, the generalized (w/o translation) chuzro as in handwritten notes.
		Base = lp_I_1
		dat = dat2b.cf0
		dbg = sol.Dcd
		Dcd.@it(dbg)
		it = 0
		#Step 0
		Base.set_basis_logical(dat)
		Dcd.@set(dbg, "iB", dat.iB)
		#Initializations
		Base.comp_cBT(dat); Base.comp_B_R(dat); Base.comp_Binv(dat);
		Base.init_β(dat); Base.init_z(dat);
		Dcd.@set(dbg, "β0", dat.β)
		#todo phaseI
		if (Base.check_feasible_β(dat) == false) println("Warning: phaseI."); fail_solution(it, sol, :Infeasible); return; end

		while(dat.maxit == 0 || it < dat.maxit)
			Dcd.@it(dbg)
			#Step 1
			Dcd.@set(dbg, "B", dat.B); Dcd.@set(dbg, "Binv", dat.Binv);
			Dcd.@set(dbg, "cBT", dat.cBT);
			Base.comp_π(dat); Dcd.@set(dbg, "π", dat.π);
			#Step 2
			Base.comp_dJ(dat); Dcd.@set(dbg, "dJ", dat.dJ);
			if Base.check_optimal_dJ(dat) Base.succeed_solution(it, dat, sol); return; end
			q = Base.price_full_dantzig(dat); Dcd.@set(dbg, "q", q);
			#Step 3
			Base.comp_αq(dat, q); Dcd.@set(dbg, "αq", dat.αq);
			#Step 4
			##########
			p,θ = Base.chuzro(dat); Dcd.@set(dbg, "p", p); Dcd.@set(dbg, "θ", θ);
			if (p == 0) Base.fail_solution(it, sol, :Unbounded); return; end
			Dcd.@set(dbg, "pivot", (q, p))
			#Step 5
			Base.pivot_iB_iR(dat, q, p)
			#Updates
			Base.update_β(dat, p, θ); Dcd.@set(dbg, "β", dat.β);
			Base.update_z(dat, q, θ); Dcd.@set(dbg, "z", dat.z);
			Base.comp_cBT(dat); Base.comp_B_R(dat); Base.comp_Binv(dat);
			it = it + 1
		end
		Base.fail_solution(it, sol, :Maxit)
	end

end

module lu_I
	#function factorize{}(B::Array{T, 2})
	#end
end

#=
module lp_I_2
	using Dcd
	using Lp

	type Dat{T}
		prob::Lp.Cf0_problem
		maxit::Int
		n::Int
		m::Int
		# Using CTSM notation.
		iB::Array{Int, 1}
		iR::Array{Int, 1}
		B::Array{T, 2}
		BL::Array{T, 2}
		BU::Array{T, 2}
		BP::Array{T, 1}
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

	function fill_dat{T}(prob::Lp.Cf0_problem, dat::Dat{T})
		n = prob.n; m = prob.m;
		dat.maxit = get(prob.params, "maxit", 0)
		dat.prob = prob
		dat.n = n
		dat.m = m
		dat.iB = Array(Int, m)
		dat.iR = Array(Int, n)
		dat.B = Array(T, (m,m))
		#dat.Binv = Array(T, (m,m))
		dat.β = Array(T, m)
		#dat.π = Array(T, m+n)
		#dat.cBT = Array(T, m)
		dat.dJ = Array(T, n)
		#dat.αq = Array(T, n)
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
	function comp_BLU{T}(dat::Dat{T}) dat.BL, dat.BU, dat.BP = lu(dat.B) end
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

	function pivot_iB_iR{T}(dat::Dat{T}, q::Int, p::Int) dat.iR[q], dat.iB[p] = dat.iB[p], dat.iR[q] end

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

	function succeed_solution{T}(dat::Dat{T}, sol::Lp.Solution)
		sol.solved = true
		sol.status = :Optimal
		sol.z = dat.z
		sol.x = eval(parse( "zeros($(dat.prob.type_s), $(dat.n))" ))
		for i = 1:length(dat.iB)
			xi = dat.iB[i]
			if (xi <= dat.n) sol.x[xi] = dat.β[i]; end
		end
	end

	function fail_solution(sol::Lp.Solution, status::Symbol) sol.solved = false; sol.status = status; end

	function solve_dat{T}(dat::Dat{T}, sol::Lp.Solution{T})
		# [CTSM].p33,p30, but with naive basis reinversion instead of update.
		dbg = sol.Dcd
		Dcd.@it(dbg)
		it = 0
		#Step 0
		set_basis_logical(dat)
		Dcd.@set(dbg, "iB", dat.iB)
		#Initializations
		comp_cBT(dat); comp_B_R(dat); comp_Binv(dat);
		init_β(dat); init_z(dat);
		Dcd.@set(dbg, "β0", dat.β)
		#todo phaseI
		if (check_feasible_β(dat) == false) println("Warning: phaseI."); fail_solution(sol, :Infeasible); return; end

		while(dat.maxit == 0 || it < dat.maxit)
			Dcd.@it(dbg)
			#Step 1
			Dcd.@set(dbg, "B", dat.B); Dcd.@set(dbg, "Binv", dat.Binv);
			Dcd.@set(dbg, "cBT", dat.cBT);
			comp_π(dat); Dcd.@set(dbg, "π", dat.π);
			#Step 2
			comp_dJ(dat); Dcd.@set(dbg, "dJ", dat.dJ);
			if check_optimal_dJ(dat) succeed_solution(dat, sol); return; end
			q = price_full_dantzig(dat); Dcd.@set(dbg, "q", q);
			#Step 3
			comp_αq(dat, q); Dcd.@set(dbg, "αq", dat.αq);
			#Step 4
			p,θ = chuzro(dat); Dcd.@set(dbg, "p", p); Dcd.@set(dbg, "θ", θ);
			if (p == 0) fail_solution(sol, :Unbounded); return; end
			Dcd.@set(dbg, "pivot", (q, p))
			#Step 5
			pivot_iB_iR(dat, q, p)
			#Updates
			update_β(dat, p, θ); Dcd.@set(dbg, "β", dat.β);
			update_z(dat, q, θ); Dcd.@set(dbg, "z", dat.z);
			comp_cBT(dat); comp_B_R(dat); comp_Binv(dat);
			it = it + 1
		end
		fail_solution(sol, :Maxit)
	end


end
=#

end #module Jad
