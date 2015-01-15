include("lp.jl")
include("arg.jl")

module lp_db
	using lp
	using lp_rsimplex_algo1
	importall arg
	
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
	end; push!(prob_db, DbProblem(problem_LPFE_p27, "LPFE_p27 degen", "", []))

	function problem_LPFE_p18(numtype)	
		return lp.create_max_problem(numtype, [-2 -1], [-1 1; -1 -2; 0 1], [-1, -2, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p18, "LPFE_p18 phaseI", "", []))

	function problem_LPFE_p20(numtype)	
		return lp.create_max_problem(numtype, [-2 -1], [-1 1; -1 -2; 0 1], [-1, -2, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p18, "LPFE_p18 phaseI", lp.Unbounded, []))

	#p184


	function solve(prob_key, arg_str::String = "")
		params = { "type" => "Float32", "dcd" => false }
		arg_get(arg_create(arg_str), params)
		
		dbprob = 0
		if (typeof(prob_key) == Int)
			dbprob = prob_db[prob_key]
		else
			for i = 1:length(prob_db)
				iprob = prob_db[i]
				if (contains(iprob.descr, prob_key))
					dbprob = iprob
					break	
				end
			end
		end

		if (typeof(dbprob) != Int)
			println()
			@printf "Problem: '%s'\n" dbprob.descr 
			println(); println("Solution:");
			lp_prob = dbprob.creator(params["type"])
			lp_prob.params["dcd"] = params["dcd"]
			sol = lp_rsimplex_algo1.solve_problem(lp_prob)
			
			used_checks = false
			if (length(dbprob.check_status) > 0)
				used_checks = true
				if (dbprob.check_status != sol.status)
					print_with_color(:red, "status: '$(sol.status)' should be '$(dbprob.check_status)' \n")
				else (dbprob.check_status != sol.status)
					print_with_color(:green, "status: '$(sol.status)' \n")	
				end
			end

			if (length(dbprob.check_x) > 0)
				used_checks = true
				if (dbprob.check_x != sol.x)
					print_with_color(:red, "x: '$(sol.x)' should be '$(dbprob.check_x)' \n")
				else
					print_with_color(:green, "x: '$(sol.x)' \n")	
				end
			end

			if (used_checks == false)
				println(sol.status)
				if (sol.solved)
					print("x: "); println(sol.x);
					print("z: "); println(sol.z);
				else
					println("nbasis:")
					lp.dcd_nbasis(sol)	
				end
			end	
			println();
			
			return sol
		end

		return lp.Solution();
	end
end
