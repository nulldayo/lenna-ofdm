%
% Shutdown SISO transmitter activities on Lyrtech Boards
%
% note: this needs to be done only when activities are finished,
%       it is perfectly valid to initialize board(s) once and work
%       all the time with a running system
%
disp('--------- RTDEX -------------------------');
VHS('Close_RTDEx_DAC');

disp('---------- VHS DAC ----------------------');
VHS('Disconnect_DAC_Board');

disp('---------- SMQ ----------------------');
SMQ('Quit');