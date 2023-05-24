function [shortPreamble_freq, longPreamble_freq, training_freq] = get_preamble_and_training(parameter)
%GET_PREAMBLE_AND_TRAINING This function defines the short term preamble,
%the long term preamble and the training data for an OFDM frame in
%frequency domain

% Short Preamble 
shortPreamble_freq_bot = [ 0, 0, 1+j, 0, 0, 0, -1-j, 0, 0, 0, 1+j, ...
0, 0, 0, -1-j, 0, 0, 0, -1-j, 0, 0, 0, 1+j, 0, 0, 0];
shortPreamble_freq_top = [ 0, 0, 0, -1-j, 0, 0, 0, -1-j, 0, 0, 0, ...
1+j, 0, 0, 0, 1+j, 0, 0, 0, 1+j, 0, 0, 0, 1+j, 0, 0];
shortPreamble_freq = zeros( 1, parameter.Nfft );

shortPreamble_freq(parameter.Nused_all_idx) = sqrt(13/6) .* ...
([shortPreamble_freq_top shortPreamble_freq_bot]).';

% Long Preamble 
longPreamble_freq_bot = [1 1 -1 -1 1 1 -1 1 -1 1 1 1 1 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1].';
longPreamble_freq_top = [1 -1 -1 1 1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1 -1 -1 1 -1 1 -1 1 1 1 1].';
longPreamble_freq = zeros( 1, parameter.Nfft );
longPreamble_freq(parameter.Nused_all_idx) = 1/sqrt(2) .* ...
([longPreamble_freq_top; longPreamble_freq_bot] + 1j .* [longPreamble_freq_top; longPreamble_freq_bot]).';
  
% Training symbols
training_freq = zeros(1, parameter.Nfft);

training_freq(parameter.Nused_all_idx) = 1/sqrt(2) .* ( [...
-1-1j, 1-1j, -1+1j, -1-1j, -1-1j, 1-1j, -1-1j, 1+1j, 1-1j, -1-1j, -1-1j, -1+1j, ...
-1-1j, -1-1j, 1-1j, -1-1j, 1-1j, -1-1j, 1+1j, -1-1j, 1+1j, -1-1j, 1-1j, 1+1j, ...
-1+1j, 1+1j, 1+1j, 1+1j, -1-1j, 1+1j, 1-1j, -1-1j, 1+1j, -1-1j, -1-1j, 1+1j, ...
1+1j, -1-1j, 1+1j, -1+1j, 1-1j, -1+1j, 1-1j, 1-1j, 1+1j, 1-1j, -1+1j, -1-1j, ...
-1-1j, -1+1j, -1+1j, 1+1j ] );

end

