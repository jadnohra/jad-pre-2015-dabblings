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
	prob_last = Array(Any, 0)

	function problem_t1(params::lp.Params)
		return lp.create_min_canonical_problem(merge(params, {"maxit" => 5}),
			[1, 1], [2 3;], [10]
			)
	end; push!(prob_db, DbProblem(problem_t1, "t1", :Optimal, :Unset, [0, 0]))

	function problem_LPFE_p88(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 5}),
			[4,3], [1 -1; 2 -1; 0 1], [1, 3, 5]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p88, "LPFE_p88", :Optimal, 31, [4, 5]))

	function problem_LPFE_p11(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 5}),
			[5,4,3], [2 3 1; 4 1 2; 3 4 2], [5, 11, 8]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p11, "LPFE_p11", :Optimal, 13, [2, 0, 1]))

	function problem_LPFE_p27(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 5}),
			[10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [0, 0, 1]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p27, "LPFE_p27 degen", :Optimal, 1, [1, 0, 1, 0]))

	function problem_LPFE_m27(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 5}),
			[10,-57,-9,-24], [0.5 -5.5 -2.5 9; 0.5 -1.5 -0.5 1; 1 0 0 0], [1.e-5, 0, 1]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_m27, "LPFE_m27 pert", :Optimal, 1, [1, 0, 1, 0]))

	function problem_LPFE_p18(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 5}),
			[-2,-1], [-1 1; -1 -2; 0 1], [-1, -2, 1]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p18, "LPFE_p18 phaseI", :Optimal, -3, [4/3, 1/3]))

	function problem_LPFE_p23_9(params::lp.Params)
		return lp.create_min_canonical_problem(merge(params, {"maxit" => 10}),
			[2,3,4], [0 2 3; 1 1 2; 1 2 3], [5, 4, 7]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p23_9, "problem_LPFE_p23_9", :Optimal, 0, [0, 0, 0]))

	function problem_LPFE_p23_8(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 10}),
			[3,2], [1 -2; 1 -1; 2 -1; 1 0; 2 1; 1 1; 1 2; 0 1], [1, 2, 6, 5, 16, 12, 21, 10]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p23_8, "problem_LPFE_p23_8", :Optimal, 28, [4, 8]))

	function problem_LPFE_p22_4(params::lp.Params)
		return lp.create_max_canonical_problem(merge(params, {"maxit" => 10}),
			[-1,-3,-1], [2 -5 1; 2 -1 2], [-5, 4]
			)
	end; push!(prob_db, DbProblem(problem_LPFE_p22_4, "problem_LPFE_p22_4", :Optimal, -3, [0, 1, 0]))

	function random_problem_impl(seed, dense, n, m, maxit, params::lp.Params)
		rng = MersenneTwister(seed)

		c = rand(rng, (n))
		b = rand(rng, (m))
		A = rand(rng, (m, n))
		if dense != 100
			#zeros = rand(rng, 1:(m*n), zero_n) # Needs next version of Julia
			zero_n = int(round((m*n) * (1.0 - dense / 100.0)))
			if (zero_n > 0)
				#sprand is useless, the output will not be the same even with fixed seed.

				xs = randn(rng, zero_n)
				for i=1:length(xs) while abs(xs[i] >= 1.0) xs[i] = rand(rng) end; end
				zeros = [ clamp(int((m*n)* 0.5*(1.0+x)), 1, (m*n) ) for x in xs]
				for i in zeros
					A[i] = 0
				end
			end
		end

		return lp.create_max_canonical_problem(merge(params, {"maxit" => maxit}),
		c, A, b
		)
	end

	function random_problem(params::lp.Params)
		seed = int(get(params, "seed", int(time_ns()) % 32768 ))
		dense = int(get(params, "dense", 100 ))
		n = int(get(params, "n", 10))
		m = int(get(params, "m", n))
		maxit = int(get(params, "maxit", 2*(m+n)))
		println("/seed:", seed)
		return random_problem_impl(seed, dense, n, m, maxit, params)
	end; push!(prob_db, DbProblem(random_problem, "random", :Unset, :Unset, :Unset))

	function seed_problem_1(params::lp.Params)
		return random_problem_impl(15578, 60, 20, 20, 2*(20+20), params)
	end; push!(prob_db, DbProblem(seed_problem_1, "seed_1", :Unset, :Unset, :Unset))

	function last_problem(params::lp.Params)
		if (length(lp_bench.prob_last) > 0)
			return lp_bench.prob_last[end]
		else
			return problem_LPFE_p88(params)
		end
	end; push!(prob_db, DbProblem(last_problem, "last", :Unset, :Unset, :Unset))

	#p184 (general)

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
			check_z = lp.conv(params["type"], dbprob.check_z)
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
			check_x = lp.conv(params["type"], dbprob.check_x)
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

	function format_percent(v)
		return strip(strip(@sprintf("%0.2f", 100.0 * v), '0'), '.') * "%"
	end

	function solve(arg_str::String = "/prob:p88", code_module = lp_I_1)
		def_params = { "prob"=>"p88", "type"=>"Float32", "dcd"=>false }
		params = deepcopy(def_params)
		arg_get(arg_create(arg_str), params)

		prob_key = params["prob"]
		if (length(prob_key) == 0 || prob_key == "all")
			for i = 1:length(prob_db) solve(i, arg_str); end;
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
			print("Problem: '", dbprob.descr, "'")
			println("\n------------")
			lp_prob = dbprob.creator(params)
			if (length(lp_bench.prob_last) > 0) pop!(lp_bench.prob_last) end
			push!(lp_bench.prob_last, deepcopy(lp_prob))
			#lp_bench.prob_last[1] =
			println("+++++++",
				" n:", lp_prob.n, ", m:", lp_prob.m,
				", density:", format_percent(lp.comp_density(lp_prob)),
				", type:", lp_prob.numtype, " +++++++")

			@time can_sol = code_module.solve_problem(lp_prob)
			println("************\n")

			sol = lp.translate_solution(lp_prob, can_sol)
			check_sol(dbprob, lp_prob, sol, params)
		end
		return sol
	end

end
