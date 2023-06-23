%% Runs all WP functions and compares them to the _p functions
run_add_paths();
%add paths first
%% -------------Initialization of different parameters---------------------
%framework function to define parameters
parameter = predefine_parameter();

% framework function to create mapping
mapping = generate_mapping(4,'QAM'); 

%framework function to import preambles and training data
[shortPreamble_freq, longPreamble_freq, training_freq] = get_preamble_and_training(parameter); 

%% --------------------------------Working packages------------------------
%WP1
lenna_symbols=import_lenna_picture_p(mapping);% Using _p function so that following functions are comparable
try
lenna_symbols_WP=import_lenna_picture(mapping);
catch ME
    warning('WP1 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    lenna_symbols_WP=0;
end
if isequal(lenna_symbols,lenna_symbols_WP)
    disp('WP1 successful');
else
    disp('WP1 unsuccessful, note that symbols might still be correct just not equal to the _p file');
end


%WP2
data_freq_matrix = rearrange_data_p(lenna_symbols,parameter);
try
data_freq_matrix_WP = rearrange_data(lenna_symbols,parameter);
catch ME
    warning('WP2 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    data_freq_matrix_WP=0;
end
if isequal(data_freq_matrix,data_freq_matrix_WP)
    disp('WP2 successful');
else
    disp('WP2 unsuccessful');
end


%WP3
data_frec_with_pilot = insert_pilot_symbols_p(data_freq_matrix,training_freq,parameter);
try
data_frec_with_pilot_WP = insert_pilot_symbols(data_freq_matrix,training_freq,parameter);
catch ME
    warning('WP3 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    data_frec_with_pilot_WP=0;
end
if isequal(data_frec_with_pilot,data_frec_with_pilot_WP)
    disp('WP3 successful');
else
    disp('WP3 unsuccessful');
end


%WP4
frames_frec_cell = create_frame_structure_p(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);
try
frames_frec_cell_WP = create_frame_structure(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);
catch ME
    warning('WP4 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    frames_frec_cell_WP=0;
end
if isequal(frames_frec_cell,frames_frec_cell_WP)
    disp('WP4 successful');
else
    disp('WP4 unsuccessful');
end


%WP5
frames_time_cell = transform_to_time_domain_p(frames_frec_cell, parameter);
try
frames_time_cell_WP = transform_to_time_domain(frames_frec_cell, parameter);
catch ME
    warning('WP5 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    frames_time_cell_WP=0;
end
if isequal(frames_time_cell,frames_time_cell_WP)
    disp('WP5 successful');
else
    disp('WP5 unsuccessful');
end


%WP6
frames = add_guard_interval_p(frames_time_cell, parameter);
try
frames_WP = add_guard_interval(frames_time_cell, parameter);
catch ME
    warning('WP6 failed critically, assigning 0');
    disp(ME.message)
    disp(ME.stack(:,1))
    frames_WP=0;
end
if isequal(frames,frames_WP)
    disp('WP6 successful');
else
    disp('WP6 unsuccessful');
end


for i=1:length(frames)%simulate ideal received frame
    frame=frames{i};
    frame(:,1:3)=[];
    rec_frame{i}=frame(:);
end

%% -------------Initialization of different parameters---------------------
%some parts are commented out because they are either identical for
%transmit and receive or because they are simulated elsewhere
% % framework function to define parameters
% parameter = predefine_parameter();
% 
% % framework function to create mapping
% mapping = generate_mapping(4,'QAM'); 
% 
% % framework function to import preambles and training data
% [shortPreamble_freq, longPreamble_freq, training_freq] = get_preamble_and_training(parameter);
% 
% % load the received frame
% load('rec_frame.mat','rec_frame');

% artificially downgrade transmission
activation_flag = true;
noise_variance = -20; % noise variance in dB
delta_F = 0.4;
rec_frame = add_noise_and_cfo(rec_frame, noise_variance, delta_F, parameter, activation_flag);

% number of received frames
Nframe = numel(rec_frame);

% number of Frames needed to transmit Lenna (assumed to be known at the receiver)
parameter.Nframes_lenna = 6;

% cell array for frames to reconstruct lenna picture
lenna_frames = cell(1,parameter.Nframes_lenna);
lenna_frames(1,:) = {zeros(length(parameter.Nused_data_idx),parameter.Nofdm-1)};

lenna_framesWP = cell(1,parameter.Nframes_lenna);
lenna_framesWP(1,:) = {zeros(length(parameter.Nused_data_idx),parameter.Nofdm-1)};

%% --------------------------------Working packages------------------------
WPflag=false(17,1);
for idx=1:Nframe   %differs from real receiver here due to "simulated"  transmission
    frame_data = rec_frame{idx};
    
    %WP7
    try
    frame_with_guardWP = rearrange_frame_data(frame_data, parameter);
    catch ME
        warning('WP7 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_with_guardWP=0;
    end
    frame_with_guard = rearrange_frame_data_p(frame_data, parameter);

    if isequal(frame_with_guardWP,frame_with_guard)
        if ~WPflag(7)
        WPflag(7)=true;
        disp('WP7 successful');
        end
    else
    disp('WP7 unsuccessful');
    end

    %WP8
    try
    FFO_estWP = estimate_FFO(frame_with_guard, parameter);
    catch ME
        warning('WP8 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        FFO_estWP=NaN;
    end
    
    FFO_est = estimate_FFO_p(frame_with_guard, parameter);
    if abs(FFO_est-FFO_estWP)<1e-4
        if ~WPflag(8)
        WPflag(8)=true;
        disp('WP8 successful');
        end
    else
    disp('WP8 unsuccessful');
    end

    %WP9
    correctFFO = true;
    try
    frame_with_guard_noFFOWP = correct_FFO(correctFFO, frame_with_guard, FFO_est, parameter);
    catch ME
        warning('WP9 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_with_guard_noFFOWP=0;
    end
    frame_with_guard_noFFO = correct_FFO_p(correctFFO, frame_with_guard, FFO_est, parameter);

    if abs(frame_with_guard_noFFO-frame_with_guard_noFFOWP)<1e-4
        if ~WPflag(9)
        WPflag(9)=true;
        disp('WP9 successful');
        end
    else
    disp('WP9 unsuccessful');
    end
    
    %WP10
    try
    frame_noGuardWP = remove_guard_interval(frame_with_guard_noFFO, parameter);
    catch ME
        warning('WP10 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_noGuardWP=0;
    end
    %
    frame_noGuard = remove_guard_interval_p(frame_with_guard_noFFO, parameter);
    if abs(frame_noGuard-frame_noGuardWP)<1e-4
        if ~WPflag(10)
        WPflag(10)=true;
        disp('WP10 successful');
        end
    else
    disp('WP10 unsuccessful');
    end

    %WP11
    try
        frame_freqWP = transform_to_frequency_domain(frame_noGuard, parameter);
    catch ME
        warning('WP11 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_freqWP=0;
    end
    %
    frame_freq = transform_to_frequency_domain_p(frame_noGuard, parameter);
    if abs(frame_freq-frame_freqWP)<1e-4
        if ~WPflag(11)
        WPflag(11)=true;
        disp('WP11 successful');
        end
    else
    disp('WP11 unsuccessful');
    end

    %WP12
    try
        channel_coefficientsWP = channel_estimation(frame_freq, training_freq);
    catch ME
        warning('WP12 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        channel_coefficientsWP=0;
    end
    %
    channel_coefficients = channel_estimation_p(frame_freq, training_freq);

    if isequal(isinf(channel_coefficients),isinf(channel_coefficientsWP))
    if abs(channel_coefficients(~isinf(channel_coefficients))-channel_coefficientsWP(~isinf(channel_coefficientsWP)))<1e-4
        if ~WPflag(12)
        WPflag(12)=true;
        disp('WP12 successful');
        end
    else
    disp('WP12 unsuccessful');
    end
    else
       disp('WP12 unsuccessful'); 
    end
    
    
    %WP13

    try
        frame_equalizedWP = equalize(frame_freq, channel_coefficients, parameter);
    catch ME
        warning('WP13 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_equalizedWP=0;
    end
    %
    frame_equalized = equalize_p(frame_freq, channel_coefficients, parameter);
    if abs(frame_equalized-frame_equalizedWP)<1e-4
        if ~WPflag(13)
        WPflag(13)=true;
        disp('WP13 successful');
        end
    else
    disp('WP13 unsuccessful');
    end
    
    %WP14
    mode = 'average';
    %mode = 'weighted_average';
    %mode = 'none';
    try
    frame_no_RFOWP = remove_RFO(frame_equalized, training_freq, channel_coefficients, parameter, mode);
    catch ME
        warning('WP14 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        frame_no_RFOWP=0;
    end
    frame_no_RFO = remove_RFO_p(frame_equalized, training_freq, channel_coefficients, parameter, mode);
    if abs(frame_no_RFO-frame_no_RFOWP)<1e-4
        if ~WPflag(14)
        WPflag(14)=true;
        disp('WP14 successful');
        end
    else
    disp('WP14 unsuccessful');
    end
    %WP15
    try
    lenna_framesWP = classify_frame(lenna_framesWP, frame_no_RFO, mapping, parameter);
    catch ME
        warning('WP15 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        lenna_framesWP(idx)={0};
    end
    lenna_frames = classify_frame_p(lenna_frames, frame_no_RFO, mapping, parameter);
     if abs(lenna_frames{idx}-lenna_framesWP{idx})<1e-4
        if ~WPflag(15)
        WPflag(15)=true;
        disp('WP15 successful');
        end
    else
    disp('WP15 unsuccessful');
    end
      
end

%WP16
try
lenna_binaryWP = demap_frames(lenna_frames, mapping);
catch ME
        warning('WP16 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        lenna_binaryWP=0;
end
lenna_binary = demap_frames_p(lenna_frames, mapping);
if isequal(lenna_binary,lenna_binaryWP)
        if ~WPflag(16)
        WPflag(16)=true;
        disp('WP16 successful');
        
        end
    else
    disp('WP16 unsuccessful');
    fprintf("Number of incorrect bits: %d\n",sum(lenna_binary~=lenna_binaryWP));
end

%WP17
try
lenna_RGBWP = reconstruct_RGB_values(lenna_binary);
catch ME
        warning('WP17 failed critically, assigning 0');
        disp(ME.message)
        disp(ME.stack(:,1))
        lenna_RGBWP=0;
end
lenna_RGB = reconstruct_RGB_values_p(lenna_binary);
if isequal(lenna_RGB,lenna_RGBWP)
        if ~WPflag(17)
        WPflag(17)=true;
        disp('WP17 successful');
        end
    else
    disp('WP17 unsuccessful');
end
imshow(lenna_RGB);

