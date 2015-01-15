# https://groups.google.com/forum/#!topic/julia-users/6p5vRSYcApY
#https://github.com/dpo/ampl.jl
#http://orfe.princeton.edu/~rvdb/ampl/nlmodels/index.html
#https://github.com/mpf/Optimization-Test-Problems (many of the same problems, without the solve command)
#http://netlib.org/ampl/models/

include("lp.jl")
include("arg.jl")

module lp_db
	using lp
	using lp_rsimplex_algo1
	importall arg

	# Waiting for https://github.com/JuliaLang/julia/pull/6884
	importall JuMP
	importall GLPKMathProgInterface

	type DbProblem
		creator::Function
		descr::String
		check_status::String
		check_x#::Array{numtype, 1}
	end

	prob_db = Any[]

	function problem_simple(numtype) 
		return lp.create_min_problem(numtype, [1, 1], [2 3;], [10], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_simple, "", "", []))

	function problem_LPFE_p88(numtype)	
		return lp.create_max_problem(numtype, [4,3], [1 -1; 2 -1; 0 1], [1, 3, 5], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p88, "LPFE_p88", lp.Optimal, [4, 5]))

	function problem_LPFE_p11(numtype)	
		return lp.create_max_problem(numtype, [5,4,3], [2 3 1; 4 1 2; 3 4 2], [5, 11, 8], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p11, "LPFE_p11", lp.Optimal, [2, 0, 1]))

	function problem_LPFE_p27(numtype)	
		return lp.create_max_problem(numtype, [10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [0, 0, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p27, "LPFE_p27 degen", lp.Optimal, [1, 0, 1, 0]))

	function problem_LPFE_m27(numtype)	
		return lp.create_max_problem(numtype, [10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [1.e-5, 0, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_m27, "LPFE_m27 pert", lp.Optimal, [1, 0, 1, 0]))

	function problem_LPFE_p18(numtype)	
		return lp.create_max_problem(numtype, [-2 -1], [-1 1; -1 -2; 0 1], [-1, -2, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p18, "LPFE_p18 phaseI", lp.Optimal, [4/3, 1/3]))

	function problem_LPFE_p23_9(numtype)	
		return lp.create_min_problem(numtype, [2 3 4], [0 2 3; 1 1 2; 1 2 3], [5, 4, 7], {"maxit" => 10})
	end; push!(prob_db, DbProblem(problem_LPFE_p23_9, "problem_LPFE_p23_9", lp.Optimal, [0, 0, 0]))

	function problem_LPFE_p23_8(numtype)	
		return lp.create_max_problem(numtype, [3 2], [1 -2; 1 -1; 2 -1; 1 0; 2 1; 1 1; 1 2; 0 1], [1, 2, 6, 5, 16, 12, 21, 10], {"maxit" => 10})
	end; push!(prob_db, DbProblem(problem_LPFE_p23_8, "problem_LPFE_p23_8", lp.Optimal, [4, 8]))

	function problem_LPFE_p22_4(numtype)	
		return lp.create_max_problem(numtype, [-1 -3 -1], [2 -5 1; 2 -1 2], [-5, 4], {"maxit" => 100})
	end; push!(prob_db, DbProblem(problem_LPFE_p22_4, "problem_LPFE_p22_4", lp.Optimal, [0, 1, 0]))


	#p184

	function check_sol(dbprob, sol, params)
		prefix = " "

		if (length(dbprob.check_status) > 0)
			if (dbprob.check_status != sol.status)
				print_with_color(:red, "$(prefix)status: '$(sol.status)' should be '$(dbprob.check_status)' \n")
			else (dbprob.check_status != sol.status)
				print_with_color(:green, "$(prefix)status: '$(sol.status)' \n")	
			end
		else	
			println("$(prefix)$(sol.status)")
		end

		if (length(dbprob.check_x) > 0)
			check_x = lp.conv_vec(params["type"], dbprob.check_x)
			if (sol.solved == false || check_x != sol.x)
				sol_x = sol.solved ? sol.x : ()
				print_with_color(:red, "$(prefix)x: '$(sol_x)' should be '$(check_x)' \n")
			else
				print_with_color(:green, "$(prefix)x: '$(sol.x)' \n")	
			end
		else
			if (sol.solved)
				println("$(prefix)x: $(sol.x)")
				println("$(prefix)z: $(sol.z)") 
			else
				if (params["dcd"] == true)
					println("$(prefix)nbasis:")
					lp.dcd_nbasis(sol)	
				end	
			end
		end

		println();
	end

	function solve(prob_key, arg_str::String = "")
		params = { "type" => "Float32", "dcd" => false, "algo" => "" }
		arg_get(arg_create(arg_str), params)
		
		dbprob = 0
		if (typeof(prob_key) == Int)
			if (prob_key == 0)
				for i = 1:length(prob_db)
					solve(i, arg_str)	
				end	
				return
			else	
				dbprob = prob_db[prob_key]
			end	
		else
			for i = 1:length(prob_db)
				iprob = prob_db[i]
				if (contains(iprob.descr, prob_key))
					dbprob = iprob
					break	
				end
			end
		end

		sol = lp.Solution()

		if (typeof(dbprob) != Int)
			println()
			@printf "Problem: '%s'\n" dbprob.descr 
			lp_prob = dbprob.creator(params["type"])
			lp_prob.params["dcd"] = params["dcd"]
			
			if (length(params["algo"]) == 0)

				println("\n------")
				@time sol = lp_rsimplex_algo1.solve_problem(lp_prob)
				println("------\n")

				check_sol(dbprob, sol, params)
			else

				m = Model(solver = GLPKSolverLP(method=:Exact))
				
				@defVar(m, lp_x[1:lp_prob.n] >= 0 )
				for ri = 1:lp_prob.m
					aff = AffExpr(lp_x[1:lp_prob.n], reshape(lp_prob.A[ri,1:lp_prob.n], lp_prob.n), 0.0)
					@addConstraint(m, aff <= lp_prob.b[ri])
				end	
				obj = AffExpr(lp_x[1:lp_prob.n], lp_prob.c, 0.0)
				setObjective(m, :Min, obj)
				
				println("\n------")
				@time status = JuMP.solve(m)
				println("------\n")

				status_dict = { :Optimal => lp.Optimal, :Unbounded => lp.Unbounded, :Infeasible => lp.Infeasible, :UserLimit => lp.Maxit, :Error => lp.Error, :NotSolved => lp.Created }
				sol = lp.Solution()
				sol.status = status_dict[status]
				sol.solved = (sol.status == lp.Optimal)
				sol.z = getObjectiveValue(m)
				if (sol.solved) 
					sol.x = eval(parse( "Array($(params["type"]), $(lp_prob.n))" ))
					for i=1:lp_prob.n
						sol.x[i] = JuMP.getValue(lp_x[i])
					end	
				end
				check_sol(dbprob, sol, params)
			end	
		end 
		return sol
	end

end
