function siso_recv_set_clock_source(rf_clock_source)
%% set RF clock source
%% author: Henryk Richter
%%
% inputs:
%  rf_clock_source 
%   'internal' - use internal RF receiver clocking
%   'external' - use external RF receiver clocking via cable 
%                to "REF OSC IN" port of quad band transceiver
% outputs:
%  -
%
%

if( 1==strcmp(rf_clock_source,'external') )
    VHS('Set_PABS_DSW_OSCS', 0, 0, 0); % OSCS = 0 external, OSCS = 1 internal
else
    VHS('Set_PABS_DSW_OSCS', 0, 0, 1); % OSCS = 0 external, OSCS = 1 internal
end

end