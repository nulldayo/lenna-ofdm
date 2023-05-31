function byte_frame = siso_trans_complex_to_byte( tx_frame, norm_constant, clip_constant )
%% convert from double valued complex input signal to integer frame
% input:  - complex valued tx_frame
%         - normalization constant (and clipping boundary if no third
%           argument is given)
%         - clipping boundary (limiter)
% output: byte array to write to Lyrtech hardware via RTDEX
% notes:
%  - the normalization constant is either any integer as desired or 0 for
%    default operation (norm by 2^15)
%  - clipping to +norm,-norm is performed implicitly

% integer normalization constant check
i16_norm = norm_constant;
if( norm_constant < 0.001 )
  i16_norm = 32768;
end
if exist('clip_constant','var')
  i16_clip = clip_constant;
else
  i16_clip = i16_norm;
end


% normalize to 16 Bit, clip to maximum extent
tx_frame_n=tx_frame.*i16_norm;
ind=find( real(tx_frame_n) > (i16_clip-1) );
tx_frame_n(ind) = i16_clip-1 + j*imag(tx_frame_n(ind));
ind=find( real(tx_frame_n) < (-i16_clip) );
tx_frame_n(ind) = -i16_clip + j*imag(tx_frame_n(ind));

ind=find( imag(tx_frame_n) > (i16_clip-1) );
tx_frame_n(ind) = j*(i16_clip-1) + real(tx_frame_n(ind));
ind=find( imag(tx_frame_n) < (-i16_clip) );
tx_frame_n(ind) = j*(-i16_clip) + real(tx_frame_n(ind));


intwarning('off');
tx_frame16=int16( tx_frame_n );
intwarning('on');

% typecast to unsigned 8 bit, merge 16 bit real and imaginary part to 
% 32 bit integer and explicitly store complex result in test frame
tx_frame8r=typecast(real(tx_frame16), 'uint8');
tx_frame8i=typecast(imag(tx_frame16), 'uint8');
byte_frame=typecast(uint32(zeros(length(tx_frame16),1)), 'uint8');
byte_frame(1:4:length(byte_frame))=tx_frame8i(1:2:length(tx_frame8i));
byte_frame(2:4:length(byte_frame))=tx_frame8i(2:2:length(tx_frame8i));
byte_frame(3:4:length(byte_frame))=tx_frame8r(1:2:length(tx_frame8r));
byte_frame(4:4:length(byte_frame))=tx_frame8r(2:2:length(tx_frame8r));

end
