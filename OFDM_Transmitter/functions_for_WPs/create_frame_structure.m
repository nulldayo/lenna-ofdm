function frames_frec_cell = create_frame_structure(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter)
%CREATE_FRAME_STRUCTURE This function creates the necessary frame structure to transmit the data. 
% Structure of one frame: [2xSTP,1xLTP,1xTraining,130xData]
% Note that the 130xData = [1xHeader,129xData] 
% -> Header: simple increasing counter for each frame -> for reconstruction
% in the correct order, since the Lenna picture cannot be transmitted by one OFDM frame
%
% Input:    data_frec_with_pilot -> 768x64 complex double
%           shortPreamble_freq -> short term preamble in frequency domain
%           longPreamble_freq -> long term preamble in frequency domain
%           training_freq -> training data in frequency domain
%           mapping -> mapping (created by generate mapping in framework)
%           parameter -> struct with predefined OFDM parameter
% Output:   frames_frec_cell -> cell array with all frames in frequence domain
%                            -> each cell contains {64x134 complex double}




end

