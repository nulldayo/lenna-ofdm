function io_frame = siso_recv_byte_to_complex( rx_frame, norm_constant )
%% convert from byte input frame to double valued complex
% input:  byte valued received frame
%         normalization constant, divider (typically 32768)
% output: output frame as complex double array
% notes:
%  - the normalization constant is either any integer as desired or 0 for
%    default operation (norm by 1/2^15)

% integer normalization constant check
i16_norm = norm_constant;
if( norm_constant < 0.001 )
  i16_norm = 32768;
end
i16_norm = 1/i16_norm; % inverse to multiply instead of divide

dout = typecast( rx_frame, 'uint8' ); % make sure uint8

% manually reorder bytes to int32, then apply negative sign
qout = int32(dout(1:4:end)) + 256.*int32(dout(2:4:end));
iout = int32(dout(3:4:end)) + 256.*int32(dout(4:4:end));
t = find(iout>32767);
iout(t) = iout(t)-65536;
t = find(qout>32767);
qout(t) = qout(t)-65536;

io_frame = i16_norm.*double(iout) + i16_norm.*j.*double(qout);

end
