function frame_no_RFO = remove_RFO(frame_equalized, training_freq, H, parameter, mode)
%REMOVE_RFO This function removes the residual frequency offset (RFO)
%
% Note: Remove unused carriers -> Only data carriers should remain
%
% Input:    frame_equalized -> 64x130 complex double
%           training_freq -> training data in frequency domain
%           H -> channel coefficients 64x1
%           parameter -> struct with predefined OFDM parameter
%           mode -> 'average', 'next' or 'none'
% Output:   frame_no_RFO -> 48x130 complex double


end

