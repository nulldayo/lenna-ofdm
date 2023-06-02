function frames = add_guard_interval(frames_time_cell, parameter)
%ADD_GUARD_INTERVAL This function adds the guard interval to the frame in time domain
%
% Input:   frames_time_cell  -> cell array with all frames in time domain
%                            -> each cell contains {64x134 complex double}
%          parameter         -> struct with predefined OFDM parameter
% Output:  frames            -> cell array with all frames in time domain with guard interval
%                            -> each cell contains {80x134 complex double}

for i=1:length(frames_time_cell)%for each frame
    iframe=frames_time_cell{i};% take i-th frame
    frames{i}=[iframe(end-parameter.Nguard+1:end,:);iframe];
end

end

