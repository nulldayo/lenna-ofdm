function frame_with_guard_noFFO = correct_FFO(correctFFO, frame_with_guard, FFO_est, parameter)
%CORRECT_FFO This function corrects the fractional frequency offset (FFO) for a given frame and FFO estimate
%
% Input:    correctFFO -> boolean to enable or disable FFO correction
%           frame_with_guard -> 80x131 complex double
%           FFO_est -> 1x1 double
%           parameter -> struct with predefined OFDM parameter
% Output:   frame_with_guard_noFFO -> 80x131 complex double
if correctFFO
N=parameter.Nfft;
% k=(1:parameter.Nofdm+1);
% i=(-parameter.Nguard:N-1).';
% l=repmat(k,N+parameter.Nguard,1)*(N+parameter.Nguard)+repmat(i,1,parameter.Nofdm+1)+1;
l=reshape(1:(80*131),80,131);
phaseshift=exp(-1j*2*pi*FFO_est*l/N);
frame_with_guard_noFFO=frame_with_guard.*phaseshift;
else
    frame_with_guard_noFFO=frame_with_guard;
end

end

