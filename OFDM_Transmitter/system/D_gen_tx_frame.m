function [ tx_frame ] = gen_tx_frame( N_frame )
%GEN_TX_FRAME Summary of this function goes here
%   Detailed explanation goes here

g_tx_os=8; % was 13
N_preamble=79;
N_periods=8;

N_data=N_frame/g_tx_os-N_preamble;

r=0.8;

% Zufallsdaten erzeugen
u=rand(1,N_data)>0.5;
x_data=1-2*u;%*exp(-j.*pi./2);

% Präamble hinzufügen
ZC_root=11; %muss Primzahl sein
preamble=zadoff_chu(N_preamble,ZC_root);

x=[preamble x_data];
x_os=upsample(x,g_tx_os);

% Sendefilterung
g=srrc(g_tx_os,r);
x_os_tx=conv(x_os,g);
tx_frame=x_os_tx(g_tx_os*N_periods+1:end);

end
