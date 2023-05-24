format long;
clear all;
close all;
clc;

% -------------------------------------------------------------------------
%
% Abtastrate AD/DA Board
%
% -------------------------------------------------------------------------
Fs = 104e6;%21e6;
Ts = 1 / Fs;

% -------------------------------------------------------------------------
%
% Paramter für das DSP-Model
%
% -------------------------------------------------------------------------
%N_frame_baseband = 1000;
%DACFactor=13;
%BaseBandFactor=8;
N_OFDM=390;    % number of ODFM data symbols (32 bit each)
N_PREAM=4;     % STP,STP,LTP,TRAINING
N_USEDCARRIER=48; % used carriers
N_BITS=2;      % Bits per carrier (QPSK)
N_CARRIERS=64; %
N_GUARD=16;    %

% Anzahl Eingangs- und Ausgangswörter vom PC und zum DAC
% Aus Flexibilitätsgründen sind hier beide gleich, damit Berechnung
% gleider Frames auf PC oder DSP möglich, wobei in letzterem Fall
% entsprechend Zero-Padding der Nutzdaten durchzuführen ist.
% Samples Input vom PC
N_FB_TX_frame = N_OFDM*32/N_USEDCARRIER/N_BITS*(N_CARRIERS+N_GUARD)+(N_PREAM*(N_CARRIERS+N_GUARD));
% Samples Output zum DAC
N_FB_RX_frame = N_OFDM*32/N_USEDCARRIER/N_BITS*(N_CARRIERS+N_GUARD)+(N_PREAM*(N_CARRIERS+N_GUARD));

% 0=no upsampling/direct, 1=2x, 2=3x,...,7=8x (ignored, use CR5 to set this
% property)
%N1_Upsampling_DAC_Factor = DACFactor-1;

% CIC Filter preset: 5x oversampling, else Sample/Hold will be applied
N1_CIC_PRESET1=5;
