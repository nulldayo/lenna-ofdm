function frames_time_cell = transform_to_time_domain(frames_frec_cell, parameter)
%TRANSFORM_TO_TIME_DOMAIN This function uses the IFFT to transform the frames to time domain
%
% Note: scale the output of the IFFT with sqrt(NFFT)
%
% Input:   frames_frec_cell  -> cell array with all frames in frequence domain
%                            -> each cell contains {64x134 complex double}
%          parameter         -> struct with predefined OFDM parameter
% Output:  frames_time_cell  -> cell array with all frames in time domain
%                            -> each cell contains {64x134 complex double}


end

