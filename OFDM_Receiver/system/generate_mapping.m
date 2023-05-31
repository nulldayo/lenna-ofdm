% #########################################################################
%
% University of Rostock
% Faculty of Informatics and Electrical Engineering
% Institute of Communications Engineering
%
% M.Sc. Stephan Schedler
% Version: #VERSION_DUMMY#
%
% #########################################################################
%
% Generates the mapping structure that is required to call the function
% "map_symbol()". If no output is assigned, a scatter plot will be shown.
%
% function [ mapping_scheme ] = generate_mapping()
% function [ mapping_scheme ] = generate_mapping(M, modulation )
% function [ mapping_scheme ] = generate_mapping(M, modulation, mapping )
% function [ mapping_scheme ] = generate_mapping(M, modulation, mapping, init_phase )
% function [                ] = generate_mapping( ... )
%
% INPUTS:
%   M                   - modulation index
%                         (default == 2)
%   modulation          - modulation scheme ('ASK'/'PSK'/'QAM'}
%                         (default == 'PSK')
%   mapping             - mapping scheme:
%                           == 'gray'               : gray mapping
%                           == 'antigray'           : anti gray mapping
%                           == 'natural'/'binary'   : natural mapping
%                           == 'rand'               : random mapping
%                         (default == 'gray')
%   init_phase          - phase rotation of symbols
%                         (default == 0)
%
% OUTPUTS:
%   mapping_scheme      - mapping structure, required by map_symbol() and
%                         demap_symbol()
%
%   EXAMPLE:
%       mapping     = generate_mapping(4, 'QAM');
%       x           = map_symbol([1 1 0 1], mapping)
%
%       mapping = generate_mapping(64, 'QAM', 'gray', 0.1*pi);
%       show_mapping(mapping);
function [ mapping_scheme ] = generate_mapping(M, modulation, mapping, init_phase )
%% Check Inputs
if ~exist('M', 'var')
    M               = 2;
end
if mod(log2(M),1) ~= 0 || M < 2
    error('The modulation index M need to be a positive power of two! (2,4,8,...)');
end
if ~exist('modulation', 'var') || isempty(modulation)
    modulation      = 'PSK';
end
if ~exist('mapping', 'var') || isempty(mapping)
    mapping         = 'gray';
end
if ~exist('init_phase', 'var') || isempty(init_phase)
    init_phase      = 0;
end


% make upper-case & remove additional letter
modulation = upper(modulation);
modulation(modulation == '-') = [];
modulation(modulation == '_') = [];
modulation(modulation == ' ') = [];

%% define modulation structure
mapping_scheme.M                    = M;                    % modulation index
mapping_scheme.m                    = log2(mapping_scheme.M);	% bits per symbol
mapping_scheme.alphabet.decimal     = 0:(2^mapping_scheme.m-1);
mapping_scheme.alphabet.binary      = de2bi_(mapping_scheme.alphabet.decimal, 'left-msb').';
mapping_scheme.alphabet.description = [num2str(M), '-', modulation,' (', mapping, ')'];


%% Determine Symbol Positions
complex = zeros(1, M);
switch upper(modulation)
    case {'ASK'}
        complex = 1:(2^mapping_scheme.m);
        complex = complex - mean(complex);
    case {'PSK'}
        for i = 0:(mapping_scheme.M-1)
            complex(i+1) = exp(1i * 2*pi*(i/mapping_scheme.M));
        end
    case {'QAM'}
        if mod(log(M)/log(4),1) ~= 0
            error('For QAM the modulation index M need to be a positive power of four! (4,16,64,...)');
        end
        
        height = sqrt(M);
        [Re, Im] = meshgrid(1:height,height:-1:1);
        Re = Re - mean(Re(:));
        Im = Im - mean(Im(:));
        complex = Re + 1i*Im;
        complex = complex(:).';
    otherwise
        error('Unsupported modulation scheme!')
end
mapping_scheme.alphabet.complex = complex;

%% normalize symbol-energy: Es = 1
 P = sum(abs(mapping_scheme.alphabet.complex).^2)/mapping_scheme.M;
 mapping_scheme.alphabet.complex = mapping_scheme.alphabet.complex / sqrt(P);

%% Determine Mapping
switch upper(mapping)
    case {'NATURAL', 'BINARY'}
        perm = 1:(2^mapping_scheme.m);
        [B,perm] = sort(perm);  
        
    case {'RAND', 'RANDOM'}
        perm =randperm(2^mapping_scheme.m);
        [B,perm] = sort(perm);  
      
    case {'GRAY'}
        if ~strcmpi(modulation, 'QAM') % not QAM
            perm = 0:(2^mapping_scheme.m-1);
            perm = bin2dec(dec2gray(perm)).';
        else %QAM
            perm = [0 2; 1 3];
            m_tmp = 2;
            while m_tmp < mapping_scheme.m
                perm = [perm+0*2^m_tmp, fliplr(perm)+2*2^m_tmp; ...
                       flipud(perm)+1*2^m_tmp, flipud(fliplr(perm))+3*2^m_tmp];
                m_tmp = m_tmp + 2;
            end
            perm = perm(:)+1;
        end
        [B,perm] = sort(perm);  
      
    case {'ANTIGRAY'}
        if ~strcmpi(modulation, 'QAM') % not QAM
            perm = antigray(2^mapping_scheme.m);
            perm = num2str(perm);
            perm(perm == ' ') = [];
            perm = reshape(perm, 2^mapping_scheme.m, mapping_scheme.m);
            perm = bin2dec(perm).';
            [B,perm] = sort(perm);       
        
        else % QAM: generated with search_QAM_antigray.m
            switch M
                case 4
                    perm = [...
                            4     3     1     2 ...
                            ];
                case 16
                    perm = [...
                            8     3    13    10     6     1    15    12    11    16     2     5     9    14     4     7 ...
                            ];
                case 64
                    perm = [...
                            38    31     3    29    47    22    40    13    19    45    41    61    54     6    50    56    16 ...
                            51     7    27    60     9    44    18    12    36    23    34    32    63    57    25    64    49 ...
                             1    55    33     4    35    20    17    43    10    52    26    15    59    24    62    58    14 ...
                            46    53    42    37    11    21    48     5    39    28     2    30     8 ...
                            ];
                otherwise
                    error('Unsupported modulation index for QAM and antigray mapping!')
            end
        end
    otherwise
        error('Unsupported mapping scheme!')
end

mapping_scheme.alphabet.complex =  mapping_scheme.alphabet.complex(perm);
%% phase rotation
 mapping_scheme.alphabet.complex =  mapping_scheme.alphabet.complex * exp(1i * init_phase);

%% show plot
if nargout == 0
    show_mapping(mapping_scheme);
end

end

%% Helper

% +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
function g = bin2gray(b)
    for j = 1:size(b,1)
        g(j,1) = b(j,1);
        for i = 2 : size(b,2);
            x = xor(str2num(b(j,i-1)), str2num(b(j,i)));
            g(j,i) = num2str(x);
        end
    end
end

% +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
function b = gray2bin(g)
    for j = 1:size(g,1)
        b(j,1) = g(j,1);
        for i = 2 : size(g,2);
            x = xor(str2num(b(j,i-1)), str2num(g(j,i)));
            b(j,i) = num2str(x);
        end
    end
end

% +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
function d = gray2dec(g)
    b = gray2bin(g);
    d = bin2dec(b);
end

% +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
function g = dec2gray(d)
    b = dec2bin(d);
    g = bin2gray(b);
end

% +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
function antigray_bins = antigray(len)
    if mod(len,2) ~= 0
        error('The lenght must be a power of two!')
    end

    if len == 2
        antigray_bins = [false; true];
    elseif len > 2
        antigray_bins = antigray(len / 2);
        tmp1 =  true(1, size(antigray_bins,1));
        tmp0 = false(1, size(antigray_bins,1));
        tmp = [tmp0;tmp1];
        tmp = tmp(:);
        
        antigray_bins = [antigray_bins; ~antigray_bins];
        antigray_bins = [tmp, antigray_bins];
    end
end

