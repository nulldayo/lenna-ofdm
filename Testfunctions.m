%% Runs all WP functions and compares them to the _p functions

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




