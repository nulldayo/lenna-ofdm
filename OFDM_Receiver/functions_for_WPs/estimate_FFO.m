function FFO_est = estimate_FFO(frame_with_guard, parameter)
%ESTIMATE_FFO This function estimates the fractional frequency offset (FFO) part of the carrier frequency offset (CFO) using the
%guard interval(Schmidl-Cox algorithm)
%
% Input:    frame_with_guard -> 80x131 complex double
%           parameter -> struct with predefined OFDM parameter
% Output:   FFO_est -> 1x1 double

FFO_arg = 0;

for k=1:size(frame_with_guard,1) % for each ofdm symbol
    for mu=1:parameter.Nguard
        FFO_arg = FFO_arg + frame_with_guard(mu,k)*(frame_with_guard(mu+parameter.Nfft,k)');
    end
end

FFO_est = (-1/(2*pi))*phase(FFO_arg);
end