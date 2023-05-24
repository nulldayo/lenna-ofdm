function [ mu ] = qam_points( M )
%QAM_POINTS Summary of this function goes here
%   Detailed explanation goes here
mu=-sqrt(M)+1:2:sqrt(M)-1;
mu=ones(sqrt(M),1)*mu+1j*mu'*ones(1,sqrt(M));
mu=mu(:);
mu=sqrt(M)*mu/sqrt(mu'*mu);


end

