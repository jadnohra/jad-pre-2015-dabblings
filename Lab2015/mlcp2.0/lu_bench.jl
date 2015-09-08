module Lu_bench
#
  importall Shared_types
  using Shared_funcs
  importall Arg
  import Lu
#
  import Lu_julia_dense
  import Lu_julia_sparse
#
  type DbProblem
    creator::Function
    descr::String
  end
#
  prob_db = DbProblem[]
  prob_last = Array(Any, 0)
#
  function problem_t1(params::Params)
    return Lu.create_Problem(params, [1 1; 1 0])
  end; push!(prob_db, DbProblem(problem_t1, "t1"))
  function problem_file(params::Params)
    println("[", params["file"], "]")
    A = Shared_funcs.read_matrix_mtx(params["file"])
    return Lu.create_Problem(params, A)
  end; push!(prob_db, DbProblem(problem_file, "file"))
  function problem_random(params::Params)
  return Lu.create_Problem(params, Shared_funcs.random_matrix(params))
  end; push!(prob_db, DbProblem(problem_random, "random"))
  function format_percent(v)
    return strip(strip(@sprintf("%0.2f", 100.0 * v), '0'), '.') * "%"
  end
  function solve_problem(code_module::Module, params::Params, lu_prob)
    T = Lu.get_t(lu_prob)
    dat = code_module.construct_dat(T)
    code_module.fill_dat(lu_prob, dat)
    sol = Lu.construct_solution(T, params)
    code_module.solve_dat(dat, sol)
    return sol
  end
  function solve_problem(params::Params, lu_prob)
    modules = {"julia_dense" => Lu_julia_dense, "julia_sparse" => Lu_julia_sparse}
    return solve_problem(modules[params["module"]], params, lu_prob)
  end
  function solve(arg_str::String = "/prob:t1")
    def_params = { "prob"=>"t1", "type"=>"Float32", "module"=>"julia_dense" }
    params = deepcopy(def_params)
    arg_get(arg_create(arg_str), params)
    #
    prob_key = params["prob"]
    if (length(prob_key) == 0 || prob_key == "all")
      for i = 1:length(prob_db) solve(i, arg_str); end;
      return
    end
    #
    found_prob = false; dbprob = 0;
    for i = 1:length(prob_db)
      iprob = prob_db[i]
      if (contains(iprob.descr, prob_key))
        found_prob = true; dbprob = iprob;
        break
      end
    end
    #
    if (found_prob)
      println()
      println("Problem: '", dbprob.descr, "'")
      println("Module: '", params["module"], "'")
      println("------------")
      lu_prob = dbprob.creator(params)
      #
      if (length(Lu_bench.prob_last) > 0) pop!(Lu_bench.prob_last) end
      push!(Lu_bench.prob_last, deepcopy(lu_prob))
      #
      println("+++++++",
        " n:", Lu.get_r(lu_prob), ", m:", Lu.get_c(lu_prob),
        ", density:", format_percent(Lu.comp_density(lu_prob)),
        ", type:", Lu.get_t(lu_prob),
        ", form:", Lu.get_form(lu_prob),
        " +++++++")
      end
      @time raw_sol = solve_problem(params, lu_prob)
      println("************")
      println("Distance: ", Lu.calc_solution_distance(lu_prob, raw_sol))
      println("")
      return raw_sol
    end
end
