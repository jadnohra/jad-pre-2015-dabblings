
module Lu
  importall Shared_types
  using Shared_funcs
	using Conv

  type Dense_problem{T}
    conv::Conv.Converter
    params::Params
    r::Int
    c::Int
    A::Matrix{T}

    Dense_problem() = new()
  end

  function get_form(prob::Dense_problem) return :Dense end
  function get_t(prob::Dense_problem) return prob.conv.t end
  function get_r(prob::Dense_problem) return prob.r end
  function get_c(prob::Dense_problem) return prob.c end


  function comp_density(prob::Dense_problem) return Shared_funcs.comp_density(prob.A) end

  function construct_Dense_problem(params::Params, A::Matrix)
    conv = Conv.converter(params["type"])
    prob = Dense_problem{conv.t}()
    prob.conv = conv; prob.params = params;
    return prob
  end

  function fill_problem{T}(params::Params, prob::Dense_problem{T}, A::Matrix)
    prob.r, prob.c = size(A)
    prob.A = Conv.matrix(prob.conv, A)
  end

  function create_Dense_problem(params::Params, A::Matrix)
    prob = construct_Dense_problem(params, A)
    fill_problem(params, prob, A)
    return prob
  end

end
