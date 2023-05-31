%--------------------------Script to define OFDM-Frames--------------------
%--------------------------------------------------------------------------
clear all;
close all;
add_paths()

%% -------------Initialization of different parameters---------------------
% framework function to define parameters
parameter = predefine_parameter();

% framework function to create mapping
mapping = generate_mapping(4,'QAM'); 

% framework function to import preambles and training data
[shortPreamble_freq, longPreamble_freq, training_freq] = get_preamble_and_training(parameter);

% load the received frame
load('rec_frame.mat','rec_frame');

% artificially downgrade transmission
activation_flag = false;
noise_variance = 50; % noise variance in dB
delta_F = 0.4;
rec_frame = add_noise_and_cfo(rec_frame, noise_variance, delta_F, parameter, activation_flag);

% number of received frames
Nframe = numel(rec_frame);

% number of Frames needed to transmit Lenna (assumed to be known at the receiver)
parameter.Nframes_lenna = 6;

% cell array for frames to reconstruct lenna picture
lenna_frames = cell(1,parameter.Nframes_lenna);
lenna_frames(1,:) = {zeros(length(parameter.Nused_data_idx),parameter.Nofdm-1)};

%% --------------------------------Working packages------------------------

for idx=10:Nframe-10   %do not use first frame
    frame_data = rec_frame{idx};
    
    %WP7
    %frame_with_guard = rearrange_frame_data(frame_data, parameter);
    frame_with_guard = rearrange_frame_data_p(frame_data, parameter);
   
    %WP8
    %FFO_est = estimate_FFO(frame_with_guard, parameter);
    FFO_est = estimate_FFO_p(frame_with_guard, parameter);
    
    %WP9
    correctFFO = true;
    %frame_with_guard_noFFO = correct_FFO(correctFFO, frame_with_guard, FFO_est, parameter);
    frame_with_guard_noFFO = correct_FFO_p(correctFFO, frame_with_guard, FFO_est, parameter);
    
    %WP10
    %frame_noGuard = remove_guard_interval(frame_with_guard_noFFO, parameter);
    frame_noGuard = remove_guard_interval_p(frame_with_guard_noFFO, parameter);
    
    %WP11
    %frame_freq = transform_to_frequency_domain(frame_noGuard, parameter);
    frame_freq = transform_to_frequency_domain_p(frame_noGuard, parameter);

    %WP12
    %channel_coefficients = channel_estimation(frame_freq, training_freq);
    channel_coefficients = channel_estimation_p(frame_freq, training_freq);
    
    %WP13
    %frame_equalized = equalize(frame_freq, channel_coefficients, parameter);
    frame_equalized = equalize_p(frame_freq, channel_coefficients, parameter);
   
    %WP14
    mode = 'average';
    %mode = 'weighted_average';
    %mode = 'none';
    %frame_no_RFO = remove_RFO(frame_equalized, training_freq, channel_coefficients, parameter, mode);
    frame_no_RFO = remove_RFO_p(frame_equalized, training_freq, channel_coefficients, parameter, mode);
    
    %WP15
    lenna_frames = classify_frame(lenna_frames, frame_no_RFO, mapping, parameter);
    %lenna_frames = classify_frame_p(lenna_frames, frame_no_RFO, mapping, parameter);
      
end

%WP16
%lenna_binary = demap_frames(lenna_frames, mapping);
lenna_binary = demap_frames_p(lenna_frames, mapping);

%WP17
%lenna_RGB = reconstruct_RGB_values(lenna_binary);
lenna_RGB = reconstruct_RGB_values_p(lenna_binary);
imshow(lenna_RGB);


