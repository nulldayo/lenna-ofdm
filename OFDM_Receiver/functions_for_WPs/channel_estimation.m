function channel_coefficients = channel_estimation(frame_freq, training_freq)
%CHANNEL_ESTIMATION This function estimates the channel coefficiants using training symbols
%
% Input:    frame_freq -> 64x131 complex double
%           training_freq -> training data in frequency domain
% Output:   channel_coefficiants -> 64x1 complex double
channel_coefficients=frame_freq(:,1)./training_freq(:);

end

