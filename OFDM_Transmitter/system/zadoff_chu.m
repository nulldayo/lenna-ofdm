function [sequence, n] = zadoff_chu(N_ZC, u)

if(~min(isprime(u)))
  % u_no_prime = u.*(~isprime(u))
   error('One u is not a prime!');
end
[x, y] = size(u);
% [~, y] = size(u);
if(y > 1)
    error('u must be a column-vector!');
end

if(u > N_ZC/2)
    error('u must be less than N_ZC/2!');
end


n = 0:N_ZC-1;

if ceil(mod(N_ZC,2))
    sequence = exp(-j*pi*u*(n.*(n+1))/N_ZC);%./sqrt(N_ZC);
else
    sequence = exp(-j*pi*u*(n.^2)/N_ZC);%./sqrt(N_ZC);
end