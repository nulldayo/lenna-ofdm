%% Runs add_paths functions in transmitter and receiver
% Not neccessary for actual implementation but useful whilst coding

function [] = run_add_paths()

root_dir = fileparts(mfilename('fullpath')); 

addpath(root_dir);

path_str = [root_dir, filesep,'OFDM_Transmitter'];                   fprintf('add path: %s\n', path_str); addpath(path_str);
mainDir=cd(path_str);add_paths();
cd(mainDir);
path_str = [root_dir, filesep,'OFDM_Receiver'];                   fprintf('add path: %s\n', path_str); addpath(path_str);
cd(path_str);add_paths();
cd(mainDir);

end