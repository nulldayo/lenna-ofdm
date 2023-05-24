function lenna_frames = classify_frame(lenna_frames, frame_no_RFO, mapping, parameter)
%CLASSIFY_FRAME This function demaps the header of one frame and organizes in a cell array it in correct order
%
% Note: Remove the header
%
% Input:    lenna_frames -> cell array with lenna frames in correct order
%           frame_no_RFO -> 48x130 complex double
%           mapping -> mapping (created by generate mapping in framework)
%           parameter -> struct with predefined OFDM parameter
% Output:   lenna_frames -> cell array with lenna frames in correct order
%                        -> a used cell contains {48x129 complex double}

end

