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
framebase=[repmat(shortPreamble_freq,2,1);longPreamble_freq;training_freq].';
N_data_frame=129;
N_frames=ceil(size(data_frec_with_pilot,1)/N_data_frame);%Number of frames needed

if ~mod(size(data_frec_with_pilot,1)/N_data_frame,1)==0 % if N_OFDM_symbols is not integer multiple of N_OFDM_symbols per frame
    data_frec_with_pilot=[data_frec_with_pilot;zeros(N_frames*N_data_frame-size(data_frec_with_pilot,1),size(data_frec_with_pilot,2))];
end
data_frec_with_pilot=data_frec_with_pilot.';
for i=1:N_frames
    header_bin=de2bi_(i,4);%using 4 bit header to align with p file, could use 2*48 bit header to allow for more frames
    header_symbol=map_symbol(header_bin,mapping);
    header=zeros(64,1);
    header(2:3)=header_symbol;% could use all Nused_data_idx for header
    header(parameter.Nused_pilot_idx)=training_freq(parameter.Nused_pilot_idx);
    frames_frec_cell{i}=[framebase,header,data_frec_with_pilot(:,((i-1)*N_data_frame+1):(i*N_data_frame))];
end


end

