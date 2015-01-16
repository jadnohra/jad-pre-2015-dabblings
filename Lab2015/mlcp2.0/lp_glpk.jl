
module lp_glpk
	using lp
	# Waiting for https://github.com/JuliaLang/julia/pull/6884
	importall JuMP
	importall GLPKMathProgInterface

	function solve_problem(lp_prob::lp.Canonical_problem)
		m = Model(solver = GLPKSolverLP(method=:Exact))
					
		@defVar(m, lp_x[1:lp_prob.n] >= 0 )
		for ri = 1:lp_prob.m
			aff = AffExpr(lp_x[1:lp_prob.n], reshape(lp_prob.A[ri,1:lp_prob.n], lp_prob.n), 0.0)
			@addConstraint(m, aff <= lp_prob.b[ri])
		end	
		obj = AffExpr(lp_x[1:lp_prob.n], lp_prob.c, 0.0)
		setObjective(m, :Min, obj)
		
		@time status = JuMP.solve(m)
	
		status_dict = { :Optimal => lp.Optimal, :Unbounded => lp.Unbounded, :Infeasible => lp.Infeasible, :UserLimit => lp.Maxit, :Error => lp.Error, :NotSolved => lp.Created }
		sol = lp.Solution()
		sol.status = status_dict[status]
		sol.solved = (sol.status == lp.Optimal)
		sol.z = getObjectiveValue(m)
		if (sol.solved) 
			sol.x = eval(parse( "Array($(lp_prob.numtype), $(lp_prob.n))" ))
			for i=1:lp_prob.n
				sol.x[i] = JuMP.getValue(lp_x[i])
			end	
		end
		return sol
	end	
end				