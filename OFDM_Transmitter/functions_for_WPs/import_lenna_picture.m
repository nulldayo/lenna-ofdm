function lenna_symbols = import_lenna_picture(mapping)
%IMPORT_LENNA_PICTURE This function reads the lenna picture and maps it
%
% Input:    mapping (created by generate mapping in framework)
% Output:   lenna_symbols -> 36864x1 complex double 

im = imread('LENNA_Scaled96.JPG','JPG'); %96x96 intensity values,0-255
imserial = im(:); %columns of im in series

imserial_bin = de2bi_(imserial); %convert to binary
imserial_bin = imserial_bin.';
imserial_bin = imserial_bin(:); %-> serialise values by line is one value

lenna_symbols = map_symbol(imserial_bin, mapping); %map binary vector to QAM symbols
lenna_symbols = lenna_symbols(:); %return lenna_symbols as column vector

end

