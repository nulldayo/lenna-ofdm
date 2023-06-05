function frame_with_guard_noFFO = correct_FFO(correctFFO, frame_with_guard, FFO_est, parameter)
%CORRECT_FFO This function corrects the fractional frequency offset (FFO) for a given frame and FFO estimate
%
% Input:    correctFFO -> boolean to enable or disable FFO correction
%           frame_with_guard -> 80x131 complex double
%           FFO_est -> 1x1 double
%           parameter -> struct with predefined OFDM parameter
% Output:   frame_with_guard_noFFO -> 80x131 complex double

if correctFFO
    for l = 1:(parameter.Nfft+parameter.Nguard)
     frame_with_guard_noFFO(l,:) = frame_with_guard(l,:)*exp(-1j*2*pi*FFO_est*l/(parameter.Nfft+parameter.Nguard));
    end
else
    frame_with_guard_noFFO = frame_with_guard;
end

end

