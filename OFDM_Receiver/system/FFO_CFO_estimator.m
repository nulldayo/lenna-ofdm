% ------------------------------------------------------------
% CFO Estimation with CP
% ------------------------------------------------------------
function [gamma_est_cp, dF_est_cp] = FFO_CFO_estimator( r_symbol, r_guard, Nfft, Nguard, Npath, dFsub, Nstop )

  if nargin < 6
    Npath = 1;
  end
  if nargin < 7
    Nstop = 0;
  end

  for k = 1:(prod(size(r_symbol)) / Nfft)
    gamma_est_cp_k(k) = ...
      -1/(2*pi) .* mean( angle( r_guard( Npath:Nguard-Nstop, k) .* ...
      conj( r_symbol( end-Nguard+Npath:end-Nstop, k ) ) ) );
    
    dF_est_cp_k(k) = gamma_est_cp_k(k) * dFsub;
  end

  dF_est_cp = mean(dF_est_cp_k);
  gamma_est_cp = mean(gamma_est_cp_k);

end
