% #########################################################################
%
% University of Rostock
% Faculty of Informatics and Electrical Engineering
% Institute of Communications Engineering
%
% #########################################################################
%
% Adds subfolders to the Matlab path.
%
% function [] = add_paths()
%
% #########################################################################

function [] = add_paths()

root_dir = fileparts(mfilename('fullpath')); 

addpath(root_dir);
path_str = [root_dir, filesep,'framework'];                   fprintf('add path: %s\n', path_str); addpath(path_str);
path_str = [root_dir, filesep,'framework', filesep,'p_code']; fprintf('add path: %s\n', path_str); addpath(path_str);
path_str = [root_dir, filesep,'functions_for_WPs'];           fprintf('add path: %s\n', path_str); addpath(path_str);

end