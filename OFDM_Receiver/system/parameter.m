format long;
clear all;
close all;
clc;
s4 = intwarning('query');
intwarning('off');

% Generate new
gen_new = 1;

% -------------------------------------------------------------------------
%
% Generelle OFDM Paramter
%
% -------------------------------------------------------------------------

% Abtastrate AD/DA Board
Fs = 20.8e6;
Ts = 1 / Fs;

% Anzahl der Subträger (Subträgerabstand ca. 328kHz)
Nfft = 64;
% Größe des Guardintervalls
Nguard = 16;

% Anzahl der ersten zu vernachlässigen Werte bei der CFO-Schätzung
% (ML-Schätzung)
Nskip = 8;

%
% SinCosGen
%
%

FreqDivider=5;  % board frequency divider (5 -> 20.8 MHz Sampling rate)
                % adjust the MA_Delay when lowering the Divider

% -------------------------------------------------------------------------
%
% Parameters for sin/cos generator
%
% -------------------------------------------------------------------------
Sincos_Latency=4; % Latency of SinCos Generator (depends on implementation 
                  % parameters in Xilinx block)

% Sin/Cos generator time resolution in Bits (4...10 makes sense, 7=default)
Sincos_Bits=7;
Sincos_Len=(2^Sincos_Bits);

% sinus/cosinus smoothing (lowpass) filter parameters (1st order Moving Average),
% cutoff frequency is Fs/FreqDivider/MA_Delay Hz
MA_Delay = 64;

% normalization constant for the input frequency specification
% (note: see sincosgen_test.m how to set the frequency, relative to system
%  clock, frequency divider and length of sin/cos table)
Theta_Bits=26;
Theta_Norm=2^Theta_Bits; % multiply phase angle theta by 2^26 (=27 Bits fractional part)



% -------------------------------------------------------------------------
%
% Trägerbelegung für das OFDM-System
%
% -------------------------------------------------------------------------

% Anzahl benutzbarer Träger (Bandbreite ca. 19.9 MHz)
Nused_all_idx   = [2:27, Nfft-25:Nfft];
Nused_all = zeros(1, Nfft);
Nused_all(Nused_all_idx) = 1;

Nused_pilot_idx = [8, 22, 44, 58];
Nused_pilot = zeros(1, Nfft);
Nused_pilot(Nused_pilot_idx) = 1;

Nused_data_idx  = setdiff(Nused_all_idx, Nused_pilot_idx);
Nused_data = zeros(1, Nfft);
Nused_data(Nused_data_idx) = 1;
Nused_data_select = { 1:6 7:19 20:24 25:29 30:42 43:48  };

% -------------------------------------------------------------------------
%
% Paramter für das DSP-Model
%
% -------------------------------------------------------------------------

% Signal-Points (QPSK = 4, 16QAM = 16, etc.)
Nmod = 4;

% Anzahl der OFDM-Symbole in einem Frame
% (130 x 48 x QPSK = 1560 Bytes > 1530 Bytes = 1 Ethernet Frame )
Nofdm         = 130;
Nofdm_bits    = Nofdm * log2(Nmod) * sum(Nused_data); % Anzahl der Bits in einem Frame bei QAM-Modulation
Nofdm_sym     = Nofdm_bits / log2(Nmod);
N_FB_RX_frame = 390;     % (390 x 32 Bits = 12480 Bits = 1530 Bytes)
N_FB_TX_frame = (Nofdm+4) * (Nfft+Nguard);


% Framelänge (Anzahl aller OFDM Symbole inkl. STP, LTP und Trainingssymbole)
Nframe = 2 + 1 + 1 + Nofdm; % 2 x STP, 1 x LTP, 1 x Training, Nofdm x Data Symbols

% Frame-Symboldauer
Tf = Ts * Nframe * (Nfft + Nguard);


if gen_new == 1
  
  % ------------------------------------------------------------
  %
  % Short Term Preamble Definition
  % Siehe "Time Synchronization Algorithms for IEEE802.11 OFDM Systems"
  % von Xiao, Cowan, Ratnarajah, Fagan
  %
  % ------------------------------------------------------------

  % Long Preamble vorbereiten
  shortPreamble_freq_bot = [ 0, 0, 1+j, 0, 0, 0, -1-j, 0, 0, 0, 1+j, ...
    0, 0, 0, -1-j, 0, 0, 0, -1-j, 0, 0, 0, 1+j, 0, 0, 0];
  shortPreamble_freq_top = [ 0, 0, 0, -1-j, 0, 0, 0, -1-j, 0, 0, 0, ...
    1+j, 0, 0, 0, 1+j, 0, 0, 0, 1+j, 0, 0, 0, 1+j, 0, 0];
  shortPreamble_freq = zeros( 1, Nfft );
  
  shortPreamble_freq(Nused_all_idx) = sqrt(13/6) .* ...
    ([shortPreamble_freq_top shortPreamble_freq_bot]).';
  
  shortPreamble_time = ifft( shortPreamble_freq, Nfft ) .* sqrt(Nfft);
  
  % ------------------------------------------------------------
  %
  % Preamble Definition
  %
  % ------------------------------------------------------------

  % Long Preamble vorbereiten
  longPreamble_freq_bot = [1 1 -1 -1 1 1 -1 1 -1 1 1 1 1 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1].';
  longPreamble_freq_top = [1 -1 -1 1 1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1 -1 -1 1 -1 1 -1 1 1 1 1].';
  longPreamble_freq = zeros( 1, Nfft );
  longPreamble_freq(Nused_all_idx) = 1/sqrt(2) .* ...
    ([longPreamble_freq_top; longPreamble_freq_bot] + 1j .* [longPreamble_freq_top; longPreamble_freq_bot]).';

  longPreamble_time = ifft( longPreamble_freq, Nfft ) .* sqrt(Nfft);
  
  tmp = sign(-(sign(real([(longPreamble_time(end:-1:end-Nguard+1)) (longPreamble_time(end:-1:1))]))-1)/2);
  longPreamble_time_real_low   = bi2de( tmp(1:32), 'right-msb');
  longPreamble_time_real_up    = bi2de( tmp(33:64), 'right-msb');
  longPreamble_time_real_guard = bi2de( tmp(65:end), 'right-msb');
  
  tmp = sign(-(sign(imag([(longPreamble_time(end:-1:end-Nguard+1)) (longPreamble_time(end:-1:1))]))-1)/2);
  longPreamble_time_imag_low   = bi2de( tmp(1:32), 'right-msb');
  longPreamble_time_imag_up    = bi2de( tmp(33:64), 'right-msb');
  longPreamble_time_imag_guard = bi2de( tmp(65:end), 'right-msb');
  
  % ------------------------------------------------------------
  %
  % Trainingssymbole definieren
  %
  % ------------------------------------------------------------
  training_freq = zeros(1, Nfft);
  training_freq(Nused_all_idx) = 1/sqrt(2) .* ( [...
    -1-1j, 1-1j, -1+1j, -1-1j, -1-1j, 1-1j, -1-1j, 1+1j, 1-1j, -1-1j, -1-1j, -1+1j, ...
    -1-1j, -1-1j, 1-1j, -1-1j, 1-1j, -1-1j, 1+1j, -1-1j, 1+1j, -1-1j, 1-1j, 1+1j, ...
    -1+1j, 1+1j, 1+1j, 1+1j, -1-1j, 1+1j, 1-1j, -1-1j, 1+1j, -1-1j, -1-1j, 1+1j, ...
    1+1j, -1-1j, 1+1j, -1+1j, 1-1j, -1+1j, 1-1j, 1-1j, 1+1j, 1-1j, -1+1j, -1-1j, ...
    -1-1j, -1+1j, -1+1j, 1+1j ] );
%   training_freq(Nused_all_idx) = 1/sqrt(2) .* ( ...
%     ( sign( randn(1,sum(Nused_all)) ) + 1j * sign( randn(1,sum(Nused_all)) ) ) );

  training_time = ifft( training_freq, Nfft ) .* sqrt(Nfft);
  
  tmp = sign(-(sign(real([(training_time(end:-1:end-Nguard+1)) (training_time(end:-1:1))]))-1)/2);
  training_time_real_low   = bi2de( tmp(1:32), 'right-msb');
  training_time_real_up    = bi2de( tmp(33:64), 'right-msb');
  training_time_real_guard = bi2de( tmp(65:end), 'right-msb');
  
  tmp = sign(-(sign(imag([(training_time(end:-1:end-Nguard+1)) (training_time(end:-1:1))]))-1)/2);
  training_time_imag_low   = bi2de( tmp(1:32), 'right-msb');
  training_time_imag_up    = bi2de( tmp(33:64), 'right-msb');
  training_time_imag_guard = bi2de( tmp(65:end), 'right-msb');

  % Wandlung Präambeln und Trainingssymbole in Integer
  training_norm = 1024;  % vor iFFT
  preamble_norm = 16384; % 2^14

  % Achtung: Tausch Real<->Imaginärteil aufgrund Little Endian auf DSP
  % Grund: Routine kopiert in 32 Bit Schritten, während Daten 2x16 Bit sind
  t = shortPreamble_time.*preamble_norm;
  i16_shortPreamble_time = int16(complex(imag(t),real(t)));
  t = longPreamble_time.*preamble_norm;
  i16_longPreamble_time = int16(complex(imag(t),real(t)));
  t = training_time.*preamble_norm;
  i16_training_time = int16(complex(imag(t),real(t)));
  
  % extract pilots
  pilot_list=[8,22,44,58];
  training_pilots=training_freq(pilot_list);
  training_pilots=training_pilots.*training_norm; % multiply by 2^8
  training_pilots_r=int16(real(training_pilots));
  training_pilots_i=int16(imag(training_pilots));

  % sin/cos tab for CFO
  sincos_norm=16384;
  sincos_num=16384;
  a=0:(sincos_num-1);sintab_15b_1024=int16(sincos_norm*sin(a*2*pi/sincos_num));costab_15b_1024=int16(sincos_norm*cos(a*2*pi/sincos_num));
  
  % training symbols in frequency domain, normalized to 32 bit
  training_freq_32Bit = training_freq .* (2^30);
  
  save('OFDM_Parameter.mat', 'Nfft', 'Nguard', 'Nskip', ...
    'Nmod', 'Nofdm', 'Nofdm_bits', 'Nofdm_sym', 'Nframe', ...
    'N_FB_RX_frame', 'N_FB_TX_frame', ...
    'Nused_all_idx', 'Nused_all', 'Nused_pilot_idx', 'Nused_pilot',...
    'Nused_data_idx', 'Nused_data', 'Nused_data_select', ...
    'shortPreamble_freq', 'shortPreamble_time', 'i16_shortPreamble_time', ...
    'longPreamble_freq', 'longPreamble_time', 'i16_longPreamble_time', ...
    'longPreamble_time_real_low', 'longPreamble_time_real_up', 'longPreamble_time_real_guard', ...
    'longPreamble_time_imag_low', 'longPreamble_time_imag_up', 'longPreamble_time_imag_guard', ...
    'training_freq', 'training_time', 'training_pilots_r', 'training_pilots_i', ...
    'training_time_real_low', 'training_time_real_up', 'training_time_real_guard', ...
    'training_time_imag_low', 'training_time_imag_up', 'training_time_imag_guard', ...
    'i16_training_time', 'sintab_15b_1024', 'costab_15b_1024', 'training_freq_32Bit' );
else
  load('OFDM_Parameter.mat');
end
intwarning(s4);