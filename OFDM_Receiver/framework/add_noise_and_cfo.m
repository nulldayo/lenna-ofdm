function out_frames = add_noise_and_cfo(frames, noise_variance_dB, delta_F, parameter, flag)
%ADD_NOISE_AND_CFO This function can add noise and cfo to the received
%frames. It can be used to artificially downgrade the channel conditions.
%
% Input:    frames -> cell array with received frames
%           noise_variance -> variance of added noise in dB
%           delta_F -> value for CFO
%           parameter -> struct with predefined OFDM parameter
%           flag -> activation flag for adding noise and cfo
%
% Output:   out_frames -> cell array with processed frames

if flag
    delta_f_0 = delta_F / parameter.T_MC;
    len_ofdm_w_guard = parameter.Nfft+parameter.Nguard;
    N_symbols = parameter.Nofdm+1;

    t = 0:parameter.T_MC/len_ofdm_w_guard:parameter.T_MC;
    t = repmat(t(1:len_ofdm_w_guard).',1,N_symbols) + repmat(0:(N_symbols)-1,len_ofdm_w_guard,1)*parameter.T_MC;

    noise_variance = 10^(noise_variance_dB/10);
    sigma_N = sqrt(noise_variance);

    out_frames = cell(1,length(frames));

    for run=1:length(frames)
       %rotate
       frame_rearranged = (reshape(frames{run}, len_ofdm_w_guard, [])); 
       y = frame_rearranged .* exp(1j*2*pi*delta_f_0*t);

       %add noise
       n = sigma_N * (randn(len_ofdm_w_guard,N_symbols)+1j*randn(len_ofdm_w_guard,N_symbols));
       res = y+n;
       out_frames{run} = res(:);
    end
else
    out_frames = frames;
end

