function frame_freq = transform_to_frequency_domain(frame_noGuard, parameter)
%TRANSFORM_TO_FREQUENCY_DOMAIN This function uses the FFT to transform a frame to frequency domain
%
% Note: scale the output of the FFT with sqrt(NFFT)
%
% Input:    frame_noGuard -> 64x131 complex double
%           parameter -> struct with predefined OFDM parameter
% Output:   frame_noGuard -> 64x131 complex double
frame_freq=fft(frame_noGuard,parameter.Nfft,1)/sqrt(parameter.Nfft);
end

