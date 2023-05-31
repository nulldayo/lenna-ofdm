function parameter = predefine_parameter()
%PREDEFINE_PARAMETER This function defines parameters for an OFDM frame

% Number of Subcarriers
parameter.Nfft = 64;

% Length of Guardinterval
parameter.Nguard = 16;

% Indices of used carriers
parameter.Nused_all_idx   = [2:27, parameter.Nfft-25:parameter.Nfft];

% Indices of pilot carriers
parameter.Nused_pilot_idx = [8, 22, 44, 58];

% Indices of data carriers
parameter.Nused_data_idx  = setdiff(parameter.Nused_all_idx, parameter.Nused_pilot_idx);

% Number of data symbols in a frame (specified due to HW)
parameter.Nofdm = 130;   

% Total number of OFDM symbols 2x STP, 1 x Preambel, 1 x Training, Nofdm x Data Symbols
parameter.N_Ofdm_in_frame = 2 + 1 + 1 + parameter.Nofdm; 

% Sampling frequancy (specified due to HW -> AD/DA Board)
parameter.Fs = 20.8e6;

% Duration of carrier (specified due to IEEE standard)
parameter.T_MC = 3.2e-6;

end

