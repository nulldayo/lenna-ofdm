function outfilename=siso_recv_search_file( infilename )
%% find requested file either in current directory or in models directory
% input:  filename without path extension
% output: filename (when found) with correct path

% current path, models subdirectory or one step back and then models/
list={ '','models/fpga/','../models/fpga/','../models/dsp/','models/dsp/','../dsp/','../fpga/' };

outfilename = infilename; % default: output input path

for i=1:size(list,2)
  tst=strcat(char(list(i)),infilename);
  if ~isempty(dir(tst)) %exist(tst,'file')
    outfilename=tst;
    break;
  end 
end

end