module Lu_julia_dense
#
	using Lu
#
	type Dat{T}
		prob::Lu.Problem{T}
		A::Array{T,2}
	#
		Dat() = new()
	end
#
	function construct_dat(T::DataType)
		return Dat{T}()
	end
	function fill_dat{T}(prob::Lu.Problem, dat::Dat{T})
		dat.prob = prob
		dat.A = full(prob.A)
	end
	function solve_dat{T}(dat::Dat{T}, sol::Lu.Solution{T})
		sol.L, sol.U, sol.p = lu(dat.A)
		sol.solved = true
	end
end
#
#
module Lu_julia_sparse
#
	using Lu
#
	type Dat{T}
		prob::Lu.Problem{T}
		A::SparseMatrixCSC
	#
		Dat() = new()
	end
#
	function construct_dat(T::DataType)
		return Dat{T}()
	end
	function fill_dat{T}(prob::Lu.Problem, dat::Dat{T})
		dat.prob = prob
		dat.A = sparse(prob.A)
	end
	function solve_dat{T}(dat::Dat{T}, sol::Lu.Solution{T})
		println(lufact(dat.A))
	end
end
