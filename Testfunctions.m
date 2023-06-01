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
catch
    warning('WP1 failed critically, assigning 0');
    lenna_symbols_WP=0;
end
if isequal(lenna_symbols,lenna_symbols_WP)
    disp('WP1 successfull');
else
    disp('WP1 unsuccessfull, note that symbols might still be correct just not equal to the _p file');
end


%WP2
data_freq_matrix = rearrange_data_p(lenna_symbols,parameter);
try
data_freq_matrix_WP = rearrange_data(lenna_symbols,parameter);
catch
    warning('WP2 failed critically, assigning 0');
    data_freq_matrix_WP=0;
end
if isequal(data_freq_matrix,data_freq_matrix_WP)
    disp('WP2 successfull');
else
    disp('WP2 unsuccessfull');
end


%WP3
data_frec_with_pilot = insert_pilot_symbols_p(data_freq_matrix,training_freq,parameter);
try
data_frec_with_pilot_WP = insert_pilot_symbols(data_freq_matrix,training_freq,parameter);
catch
    warning('WP3 failed critically, assigning 0');
    data_frec_with_pilot_WP=0;
end
if isequal(data_frec_with_pilot,data_frec_with_pilot_WP)
    disp('WP3 successfull');
else
    disp('WP3 unsuccessfull');
end


%WP4
frames_frec_cell = create_frame_structure_p(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);
try
frames_frec_cell_WP = create_frame_structure(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);
catch
    warning('WP4 failed critically, assigning 0');
    frames_frec_cell_WP=0;
end
if isequal(frames_frec_cell,frames_frec_cell_WP)
    disp('WP4 successfull');
else
    disp('WP4 unsuccessfull');
end


%WP5
frames_time_cell = transform_to_time_domain_p(frames_frec_cell, parameter);
try
frames_time_cell_WP = transform_to_time_domain(frames_frec_cell, parameter);
catch
    warning('WP5 failed critically, assigning 0');
    frames_time_cell_WP=0;
end
if isequal(frames_time_cell,frames_time_cell_WP)
    disp('WP5 successfull');
else
    disp('WP5 unsuccessfull');
end


%WP6
frames = add_guard_interval_p(frames_time_cell, parameter);
try
frames_WP = add_guard_interval(frames_time_cell, parameter);
catch
    warning('WP6 failed critically, assigning 0');
    frames_WP=0;
end
if isequal(frames,frames_WP)
    disp('WP6 successfull');
else
    disp('WP6 unsuccessfull');
end


