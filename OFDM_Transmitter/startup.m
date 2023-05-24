% Dieses Script-File uebernimmt alle Notwendigen Verzeichnisseinstellungen.
% Es muss als erstes ausgefuehrt werden, damit saemtliche Funktionen
% gefunden und aufgerufen werden koennen.

% Für Octave
%addpath(genpath("/usr/share/octave/site/api-v22/m/"));
%fprintf('Setze Pfadeintraege:\n\n');

cursearchpath=path();
curloc=pwd();

str='/system';
curfile=fullfile(curloc,str);
k=strfind(cursearchpath, curfile);
if( isempty(k) )
  addpath([pwd str]);
end
str='/lyrtech';
curfile=fullfile(curloc,str);
k=strfind(cursearchpath, curfile);
if( isempty(k) )
  addpath([pwd str]);
end
str='/models';
curfile=fullfile(curloc,str);
k=strfind(cursearchpath, curfile);
if( isempty(k) )
  addpath([pwd str]);
end

parameter();