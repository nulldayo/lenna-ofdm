% Kanalschaetzung unter der Nutzung von Referenzdaten mit der Cross
% Correlation Methode oder dem Least Squares Algorithmus
%
% function h_hat = ref_KS (fk, vk, q_hat[, X_flag]);
% ----------------------------------------------------------------------
% INPUT:
% ----------------------------------------------------------------------
% fk       : Spalten- oder Zeilenvektor der gesendeten Referenzdaten 
%            f(k) im Symboltakt k. f(k) kann wahlweise aus {0,1} oder 
%            {-1,1} bestehen. Diese werden stets in eine {-1,1}-Folge 
%            umgewandelt.
% vk       : empfangene Midambel im Symboltakt k am Ausgang der 
%            Derotation. Die Laenge der Midambel muss mit der der 
%            Referenzdaten uebereinstimmen. 
% q_hat    : Grad der zu schaetztenden Kanal-Impulsantwort h_hat
%      N.B.: Für das Korrelationsverfahren gilt: q_hat < 6 !! 
% [X_flag] : optionaler Parameter ueber die Art der referenzdatenge-
%            stuetzten Kanalschaetzung:
%            0: Least Squares Algorithmus (LS) (default)
%            1: Korrelationsverfahren (CC)
% ----------------------------------------------------------------------
% OUTPUT:
% ----------------------------------------------------------------------
% h_hat    : Spaltenvektor der geschaetzten Kanalimpulsantwort der 
%            Ordnung q_hat im Symboltakt k (Laenge: q_hat + 1)
% ----------------------------------------------------------------------
% REMARKS:
% ----------------------------------------------------------------------
% Quellen  : - Frank Jordan
% ----------------------------------------------------------------------
% AUTHORS:
% ----------------------------------------------------------------------
% Thorsten Petermann                             24.07.97 (last update)
% ----------------------------------------------------------------------

% <---------------------------- max. linewidth for atops: 100 ch. ------------------------------->|

function h_hat = ref_KS (fk, vk, q_hat, X_flag)

if nargin < 4
  X_flag = 0;
end

vk = vk(:);
L_vk = length(vk);
fk = fk(:);
L_fk = length(fk);

if length(fk) ~= length(vk)
  error('ERROR (ref_KS.m): Referenzdatenbereich muss 26 Bits enthalten!');
end;
if (X_flag==1) & (q_hat>5)
  error(sprintf('ERROR (ref_KS.m): Für CC-Verfahren ist q_hat = %d zu groß gewählt!', q_hat));
end

[pos,fk_tmp] = find(fk==0);                % Umwandlung von {0,1}- in {-1,1}-Symbole       
fk(pos) = -fk_tmp;

if X_flag == 0                             % ===== LS-Algorithmus
  fk_mat = toeplitz(fk(q_hat+1:L_fk), fk(q_hat+1:-1:1));
  h_hat = fk_mat\vk(q_hat+1:L_vk);         % h_hat = inv(fk_mat'*fk_mat)*fk_mat'*vk
  if size(fk_mat,1) <= size(fk_mat,2)
    disp('WARNUNG (ref_KS.m): Fehlschaetzung moeglich, da q_hat fuer LS zu gross gewaehlt!');
  end
 
elseif X_flag == 1                         % ===== Korrelationsverfahren (CC)
  kern = 6:21;                             % Referenzdaten-Kern (16 Bit)
  mk = fk(kern);                           % orthogonale Sequenz m(k) = [f(6),...,f(21)]^T 
  L_mk = length(mk);
  mk_mat = toeplitz([mk(1); zeros(q_hat,1)], [mk; zeros(q_hat,1)]);
  h_hat = mk_mat*vk(min(kern):max(kern)+q_hat)/L_mk;
                                           % Bestimmung der Koeffizienten der geschaetzten...
                                           % Kanalimpulsantwort durch skalare Multiplikation...
                                           % des Referenzdatenkerns mit der jeweils um ein...
                                           % Element nach links verschobenen Midambel
end

% -EOF-
