%fi = VHS('Read_User_Reg_ADC', 3);
c = VHS('Read_User_Reg_ADC', 0);
b = VHS('Read_User_Reg_ADC', 6);
a = VHS('Read_User_Reg_ADC', 5);
fprintf('VHS STATE %d IDLE %d WRITE %d READ %d FIFO RST %d FULL %d EMPTY %d\nRCHREADY %d DSPFree %d DSPBusy %d ReturnFrame %d FIFO_DCount %d RecFrameCount %d\n\n',...
    c,bitand(a,1),...
    bitand(a,2)/2,...
    bitand(a,4)/4,...
    bitand(a,8)/8,...
    bitand(a,16)/16,...
    bitand(a,32)/32,...
    bitand(a,64)/64,...
    bitand(a,128)/128,...
    bitand(a,256)/256,...
    bitand(a,512)/512,...
    bitshift(a,-10),b... % -7
    );
