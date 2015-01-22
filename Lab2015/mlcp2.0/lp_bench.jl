#https://groups.google.com/forum/#!topic/julia-users/6p5vRSYcApY
#https://github.com/dpo/ampl.jl
#http://orfe.princeton.edu/~rvdb/ampl/nlmodels/index.html
#https://github.com/mpf/Optimization-Test-Problems (many of the same problems, without the solve command)
#http://netlib.org/ampl/models/


module lp_bench
	using lp
	using jad.lp_I_1
	importall arg

	type DbProblem
		creator::Function
		descr::String
		check_status::Symbol
		check_z#::Array{numtype, 1}
		check_x#::Array{numtype, 1}
	end

	prob_db = Any[]

	function problem_t1(numtype, params::lp.Params)
		return lp.create_min_canonical_problem(numtype, [1, 1], [2 3;], [10], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_t1, "t1", :Optimal, :Unset, [0, 0]))

	function problem_LPFE_p88(numtype)
		return lp.create_max_canonical_problem(numtype, [4,3], [1 -1; 2 -1; 0 1], [1, 3, 5], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p88, "LPFE_p88", :Optimal, 31, [4, 5]))

	function problem_LPFE_p11(numtype)
		return lp.create_max_canonical_problem(numtype, [5,4,3], [2 3 1; 4 1 2; 3 4 2], [5, 11, 8], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p11, "LPFE_p11", :Optimal, :Unset, [2, 0, 1]))

	function problem_LPFE_p27(numtype)
		return lp.create_max_canonical_problem(numtype, [10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [0, 0, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p27, "LPFE_p27 degen", :Optimal, :Unset, [1, 0, 1, 0]))

	function problem_LPFE_m27(numtype)
		return lp.create_max_canonical_problem(numtype, [10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [1.e-5, 0, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_m27, "LPFE_m27 pert", :Optimal, :Unset, [1, 0, 1, 0]))

	function problem_LPFE_p18(numtype)
		return lp.create_max_canonical_problem(numtype, [-2 -1], [-1 1; -1 -2; 0 1], [-1, -2, 1], {"maxit" => 5})
	end; push!(prob_db, DbProblem(problem_LPFE_p18, "LPFE_p18 phaseI", :Optimal, :Unset, [4/3, 1/3]))

	function problem_LPFE_p23_9(numtype)
		return lp.create_min_canonical_problem(numtype, [2 3 4], [0 2 3; 1 1 2; 1 2 3], [5, 4, 7], {"maxit" => 10})
	end; push!(prob_db, DbProblem(problem_LPFE_p23_9, "problem_LPFE_p23_9", :Optimal, :Unset, [0, 0, 0]))

	function problem_LPFE_p23_8(numtype)
		return lp.create_max_canonical_problem(numtype, [3 2], [1 -2; 1 -1; 2 -1; 1 0; 2 1; 1 1; 1 2; 0 1], [1, 2, 6, 5, 16, 12, 21, 10], {"maxit" => 10})
	end; push!(prob_db, DbProblem(problem_LPFE_p23_8, "problem_LPFE_p23_8", :Optimal, :Unset, [4, 8]))

	function problem_LPFE_p22_4(numtype)
		return lp.create_max_canonical_problem(numtype, [-1 -3 -1], [2 -5 1; 2 -1 2], [-5, 4], {"maxit" => 100})
	end; push!(prob_db, DbProblem(problem_LPFE_p22_4, "problem_LPFE_p22_4", :Optimal, :Unset, [0, 1, 0]))

	function random_problem(numtype)
		return lp.create_max_canonical_problem(numtype, [-1 -3 -1], [2 -5 1; 2 -1 2], [-5, 4], {"maxit" => 100})
	end; push!(prob_db, DbProblem(problem_LPFE_p22_4, "random", :Optimal, :Unset, [0, 1, 0]))

	#p184

	function check_sol(dbprob, lp_prob, sol, params)

		prefix = " "
		if (dbprob.check_status != :Unset)
			if (dbprob.check_status != sol.status)
				print_with_color(:red, "$(prefix)status: '$(sol.status)' should be '$(dbprob.check_status)' \n")
			else (dbprob.check_status != sol.status)
				print_with_color(:green, "$(prefix)status: '$(sol.status)' \n")
			end
		else
			println("$(prefix)$(sol.status)")
		end

		if (dbprob.check_z != :Unset)
			check_z = lp.conv_scalar(params["type"], dbprob.check_z)
			if (check_z != sol.z)
				print_with_color(:red, "$(prefix)z: '$(sol.z)' should be '$(check_z)' \n")
			else
				print_with_color(:green, "$(prefix)z: '$(sol.z)' \n")
			end
		else
			if (sol.solved)
				println("$(prefix)z: $(sol.z)")
			end
		end

		if (dbprob.check_x != :Unset)
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
			else
				if (params["dcd"] == true)
					println("$(prefix)nbasis:")
					lp.dcd_nbasis(sol)
				end
			end
		end

		println();
	end

	function solve(arg_str::String = "/prob:p88", code_module = lp_I_1)
		params = { "prob" => "p88", "type" => "Float32", "dcd" => false }
		arg_get(arg_create(arg_str), params)

		prob_key = params["prob"]
		if (length(prob_key) == 0 || prob_key == "all")
			@time for i = 1:length(prob_db) solve(i, arg_str); end;
			return
		end

		found_prob = false; dbprob = 0;
		for i = 1:length(prob_db)
			iprob = prob_db[i]
			if (contains(iprob.descr, prob_key))
				found_prob = true; dbprob = iprob;
				break
			end
		end

		if (found_prob)
			println()
			@printf "Problem: '%s'\n" dbprob.descr
			lp_prob = dbprob.creator(params["type"])
			lp_prob.params["dcd"] = params["dcd"]

			println("\n------")
			can_sol = code_module.solve_problem(lp_prob)
			println("------\n")

			sol = lp.translate_solution(lp_prob, can_sol)
			check_sol(dbprob, lp_prob, sol, params)
		end
		return sol
	end

end
