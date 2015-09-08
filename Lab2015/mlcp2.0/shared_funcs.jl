module Shared_funcs
  # Waiting for https://github.com/JuliaLang/julia/pull/6884
  using GZip
  function comp_density(A::AbstractMatrix)
    return 1.0 - (countnz(A) / length(A))
  end
  function _read_matrix_mtx(filename, infoonly::Bool=false)
    function skewsymmetric!(M::AbstractMatrix)
        m,n = size(M)
        m == n || throw(DimensionMismatch())
        for i=1:n, j=1:n
            if M[i,j] != 0
                M[j,i] = -M[i,j]
            end
        end
        return M
    end
    function symmetric!(M::AbstractMatrix)
        m,n = size(M)
        m == n || throw(DimensionMismatch())
        for i=1:n, j=1:n
            if M[i,j] != 0
                M[j,i] = M[i,j]
            end
        end
        return M
    end
    function hermitian!(M::AbstractMatrix)
        m,n = size(M)
        m == n || throw(DimensionMismatch())
        for i=1:n, j=1:n
            if M[i,j] != 0
                M[j,i] = conj(M[i,j])
            end
        end
        return M
    end
    #println(pwd())
    mmfile = Nothing()
    if (endswith(filename, ".gz"))
      mmfile = GZip.open(filename,"r")
    else
      mmfile = open(filename,"r")
    end
    # Read first line
    firstline = chomp(readline(mmfile))
    tokens = split(firstline)
    if length(tokens) != 5
      throw(ParseError(string("Not enough words on first line: ", ll)))
    end
    if tokens[1] != "%%MatrixMarket"
      throw(ParseError(string("Not a valid MatrixMarket header:", ll)))
    end
    (head1, rep, field, symm) = map(lowercase, tokens[2:5])
    if head1 != "matrix"
        throw(ParseError("Unknown MatrixMarket data type: $head1 (only \"matrix\" is supported)"))
    end
    eltype = field == "real" ? Float64 :
             field == "complex" ? Complex128 :
             field == "pattern" ? Bool :
             throw(ParseError("Unsupported field $field (only real and complex are supported)"))

    symlabel = symm == "general" ? identity :
               symm == "symmetric" ? symmetric! :
               symm == "hermitian" ? hermitian! :
               symm == "skew-symmetric" ? skewsymmetric! :
               throw(ParseError("Unknown matrix symmetry: $symm (only general, symmetric, skew-symmetric and hermitian are supported)"))
    # Skip all comments and empty lines
    ll   = readline(mmfile)
    while length(chomp(ll))==0 || (length(ll) > 0 && ll[1] == '%')
        ll = readline(mmfile)
    end
    # Read matrix dimensions (and number of entries) from first non-comment line
    dd = map(parseint, split(ll))
    if length(dd) < (rep == "coordinate" ? 3 : 2)
        throw(ParseError(string("Could not read in matrix dimensions from line: ", ll)))
    end
    rows = dd[1]
    cols = dd[2]
    entries = (rep == "coordinate") ? dd[3] : (rows * cols)
    if infoonly
        return (rows, cols, entries, rep, field, symm)
    end
    if rep == "coordinate"
        rr = Array(Int, entries)
        cc = Array(Int, entries)
        xx = Array(eltype, entries)
        for i in 1:entries
            flds = split(readline(mmfile))
            rr[i] = parseint(flds[1])
            cc[i] = parsefloat(flds[2])
            if eltype == Complex128
                xx[i] = Complex128(parsefloat(flds[3]), parsefloat(flds[4]))
            elseif eltype == Float64
                xx[i] = parsefloat(flds[3])
            else
                xx[i] = true
            end
        end
        return symlabel(sparse(rr, cc, xx, rows, cols))
    end
    return symlabel(reshape([parsefloat(readline(mmfile)) for i in 1:entries], (rows,cols)))
  end
  function read_matrix_mtx(filename)
    return _read_matrix_mtx(filename)
  end
  function random_matrix(seed, scale, dense, n, m, show)
    rng = MersenneTwister(seed)
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
          A[i] = A[i]*0
        end
      end
    end
    if (show)
      println("A", A)
    end
    return A
  end
  function random_matrix(params)
    seed = int(get(params, "seed", int(time_ns()) % 32768))
    scale = int(get(params, "scale", 1.0))
    dense = int(get(params, "dense", 100 ))
    n = int(get(params, "n", 10))
    m = int(get(params, "m", n))
    show = get(params, "show", false)
    println("/seed:", seed)
    return random_matrix(seed, scale, dense, n, m, show)
  end
end
