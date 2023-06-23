function lenna_RGB = reconstruct_RGB_values(lenna_binary)
%RECONSTRUCT_RGB_VALUES This function reconstructs RGB values of lenna from the binary data vector
%
% Note: - use uint8 for decimal values -> 8 bit for one desimal number with 'left-msb'
%       - The original picture size is 96x96        
%
% Input:    lenna_binary -> 1x74304 logical           
% Output:   lenna_RGB -> 96x96 uint8

lenna_binary=reshape(lenna_binary(:),8,[]).';
lenna_uint8=uint8(bi2de_(lenna_binary));
lenna_uint8(96*96+1:end)=[];
lenna_RGB=reshape(lenna_uint8,96,96);


end

