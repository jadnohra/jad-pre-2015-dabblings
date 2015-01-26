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

	function random_problem_impl(seed, scale, dense, n, m, maxit, show, params::lp.Params)
		rng = MersenneTwister(seed)

		c = scale * rand(rng, (n))
		b = scale * rand(rng, (m))
		A = scale * rand(rng, (m, n))
		if dense != 100
			#zeros = rand(rng, 1:(m*n), zero_n) # Needs next version of Julia
			zero_n = int(round((m*n) * (1.0 - dense / 100.0)))
			if (zero_n > 0)
				#sprand is useless, the output will not be the same even with fixed seed.
				toindex = x -> clamp(int((m*n)* 0.5*(1.0+x)), 1, m*n)
				xis = [ toindex(x) for x in randn(rng, zero_n) ]
				for i=2:length(xis)
					it = 0
					while (it < 5 && xis[i] in xis[1:i-1])
						xis[i] = toindex(rand(rng))
						it = it + 1
					end
				end
				for i in xis
					A[i] = 0
				end
			end
		end

		if (show)
			println("c", c)
			println("b", b)
			println("A", A)
		end

		return lp.create_max_canonical_problem(merge(params, {"maxit" => maxit}),
		c, A, b
		)
	end

	function random_problem(params::lp.Params)
		seed = arg.dict_get(params, "seed", int(time_ns()) % 32768 )
		scale = int(arg.dict_get(params, "scale", 1.0))
		dense = arg.dict_get(params, "dense", 100 )
		n = arg.dict_get(params, "n", 10)
		m = arg.dict_get(params, "m", n)
		maxit = arg.dict_get(params, "maxit", 2*(m+n))
		show = arg.dict_get(params, "show", false)
		println("/seed:", seed)
		return random_problem_impl(seed, scale, dense, n, m, maxit, show, params)
	end; push!(prob_db, DbProblem(random_problem, "random", :Unset, :Unset, :Unset))

	function seed_problem_1(params::lp.Params)
		return random_problem_impl(15578, 1, 60, 20, 20, 2*(20+20), false, params)
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
			check_z = lp.conv(lp.conv_func(params["type"]), dbprob.check_z)
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
			check_x = lp.conv(lp.conv_func(params["type"]), dbprob.check_x)
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

		if (sol.iter != 0)
			println("$(prefix)it: $(sol.iter)")
		end

		println();
	end

	function format_percent(v)
		return strip(strip(@sprintf("%0.2f", 100.0 * v), '0'), '.') * "%"
	end

	function introspect(arg_str::String = "", code_module = lp_I_1)
		def_params = { "type"=>"Float32", "kind"=>"native" }
		params = deepcopy(def_params)
		arg_get(arg_create(arg_str), params)


		function do_introspect(kind, out, code_func, intr_func, intr_arg_types)
			str = ""
			if (out == :Str)
				str = join(code_func(intr_func, intr_arg_types))
			else
				orig_stdout = STDOUT
				try
					(outRead, outWrite) = redirect_stdout()
					code_func(intr_func, intr_arg_types)
					str = readavailable(outRead)
				catch exc
					redirect_stdout(orig_stdout)
					println(exc)
				end
				redirect_stdout(orig_stdout)
			end
			println("--------------: ", kind)
			println(str)
			nlines = length(matchall(r"\n", str))
			println("--------------> ", nlines, " lines")
		end

		code_info_map = {"typed" => (:Str, code_typed), "lowered" => (:Str, code_lowered), "llvm" => (:Stdout, code_llvm), "native" => (:Stdout, code_native) }
		kind = params["kind"]
		intr_func = code_module.solve_problem
		numtype = params["type"]
		solve_arg_types = ( eval(parse("lp.Canonical_problem{$(numtype)}")), )
		intr_arg_types = solve_arg_types

		# refactor
		intr_func = code_module.solve_dat
		intr_arg_types = ( eval(parse("lp_I_1.Dat{$(numtype)}")), lp.Solution, )


		if (kind != "all")
			code_info = code_info_map[params["kind"]]
			do_introspect(kind, code_info[1], code_info[2], intr_func, intr_arg_types)
		else
			for k in code_func_map
				code_info = code_info_map[k]
				do_introspect(code_info[1], code_info[2], intr_func, intr_arg_types)
			end
		end
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
			#Profile.print()
			if (length(lp_bench.prob_last) > 0) pop!(lp_bench.prob_last) end
			push!(lp_bench.prob_last, deepcopy(lp_prob))
			#lp_bench.prob_last[1] =
			println("+++++++",
				" n:", lp_prob.n, ", m:", lp_prob.m,
				", density:", format_percent(lp.comp_density(lp_prob)),
				", type:", lp_prob.type_s, " +++++++")

			@time can_sol = code_module.solve_problem(lp_prob)
			println("************\n")

			sol = lp.translate_solution(lp_prob, can_sol)
			check_sol(dbprob, lp_prob, sol, params)
		end
		return sol
	end

end
