function FFO_est = estimate_FFO(frame_with_guard, parameter)
%ESTIMATE_FFO This function estimates the fractional frequency offset (FFO) part of the carrier frequency offset (CFO) using the
%guard interval(Schmidl-Cox algorithm)
%
% Input:    frame_with_guard -> 80x131 complex double
%           parameter -> struct with predefined OFDM parameter
% Output:   FFO_est -> 1x1 double
guard=frame_with_guard(1:parameter.Nguard,:);
frame_end=frame_with_guard(end-parameter.Nguard+1:end,:);
FFO_est=-1/(2*pi)*phase(sum(guard.*conj(frame_end),'all'));

end

