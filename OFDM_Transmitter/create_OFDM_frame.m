%clear all
%close all
%--------------------------Script to define OFDM-Frames--------------------
%--------------------------------------------------------------------------
add_paths()

%% -------------Initialization of different parameters---------------------
%framework function to define parameters
parameter = predefine_parameter();

% framework function to create mapping
mapping = generate_mapping(4,'QAM'); 

%framework function to import preambles and training data
[shortPreamble_freq, longPreamble_freq, training_freq] = get_preamble_and_training(parameter); 

%% --------------------------------Working packages------------------------
%WP1
lenna_symbols = import_lenna_picture_p(mapping);
%lenna_symbols = import_lenna_picture(mapping);

%WP2
data_freq_matrix = rearrange_data_p(lenna_symbols,parameter);
%data_freq_matrix = rearrange_data(lenna_symbols,parameter);

%WP3
data_frec_with_pilot = insert_pilot_symbols_p(data_freq_matrix,training_freq,parameter);
%data_frec_with_pilot = insert_pilot_symbols(data_freq_matrix,training_freq,parameter);

%WP4
frames_frec_cell = create_frame_structure_p(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);
%frames_frec_cell = create_frame_structure(data_frec_with_pilot,shortPreamble_freq, longPreamble_freq, training_freq, mapping, parameter);

%WP5
frames_time_cell = transform_to_time_domain_p(frames_frec_cell, parameter);
%frames_time_cell = transform_to_time_domain(frames_frec_cell, parameter);

%WP6
frames = add_guard_interval_p(frames_time_cell, parameter);
%frames = add_guard_interval(frames_time_cell, parameter);


