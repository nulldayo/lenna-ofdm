function lenna_binary = demap_frames(lenna_frames, mapping)
%DEMAP_FRAMES This function concatenates the frames in correct order and demaps the symbols to binary data
%
% Input:    lenna_frames -> cell array with lenna frames in correct order
%           mapping -> mapping (created by generate mapping in framework)
% Output:   lenna_binary -> 1x74304 logical
lenna_binary=false(1,mapping.m*numel(lenna_frames{1})*length(lenna_frames));
for i=1:length(lenna_frames)
    frame=lenna_frames{i};
    frame=frame(:); % convert frame matrix to column vector
    frame_bin=demap_symbol(frame,mapping);
    lenna_binary(1,(i-1)*mapping.m*numel(lenna_frames{1})+1:i*mapping.m*numel(lenna_frames{1}))=frame_bin;
end

end

