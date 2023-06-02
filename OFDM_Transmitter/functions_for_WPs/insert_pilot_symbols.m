function data_frec_with_pilot = insert_pilot_symbols(data_freq_matrix,training_freq,parameter)
%INSERT_PILOT_SYMBOLS This function inserts training symbols as pilot symbols to OFDM symbols at the specific carriers
%
% Input:    data_freq_matrix -> 768x64 complex double
%           training_freq -> training data in frequency domain
%           parameter -> struct with predefined OFDM parameter
% Output:   data_frec_with_pilot -> 768x64 complex double
data_freq_matrix(:,parameter.Nused_pilot_idx)=repmat(training_freq(parameter.Nused_pilot_idx),size(data_freq_matrix,1),1);
data_frec_with_pilot=data_freq_matrix;
end

