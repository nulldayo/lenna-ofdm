function data_freq_matrix = rearrange_data(lenna_symbols,parameter)
%REARRANGE_DATA This function rearranges the data to the OFDM symbol-structure using 64 carriers
%
% Input:    lenna_symbols -> 36864x1 complex double 
%           parameter -> struct with predefined OFDM parameter
% Output:   data_freq_matrix -> 768x64 complex double

N_OFDM_symbols=ceil(length(lenna_symbols)/length(parameter.Nused_data_idx));% check how many OFDM symbols are needed to transmit all symbols
data_freq_matrix=zeros(N_OFDM_symbols,parameter.Nfft);

if ~mod(length(lenna_symbols)/length(parameter.Nused_data_idx),1)==0 % If N lenna symbols is not integer multiple of symbols per OFDM symbol
    lenna_symbols=[lenna_symbols;zeros(N_OFDM_symbols*length(parameter.Nused_data_idx)-length(lenna_symbols),1)];% Add Zeros to adjust length
end
data_freq_matrix(:,parameter.Nused_data_idx)=reshape(lenna_symbols,length(parameter.Nused_data_idx),N_OFDM_symbols).';% Insert lenna_symbols into matrix rows at specified columns
                                                                                                                      % Note: reshape fills by column -> transpose and swapped reshape arguments
end

