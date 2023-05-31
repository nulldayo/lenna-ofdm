function sel = state_machine_fifos(RST, FIFO_Count0, FIFO_Count1, N_FB_TX_frame, N_FB_TX_frameTot )

  persistent state, state = xl_state(0, {xlUnsigned,1,0});
  
  if RST
    state = 0; % erster FIFO wird gelesen, zweiter geschrieben
  % erster FIFO leer, zweiter voll ?
  elseif state==0 && FIFO_Count0 == 0 && FIFO_Count1 == N_FB_TX_frame
    state = 1;
  elseif state==0 && FIFO_Count0 == 0 && FIFO_Count1 == N_FB_TX_frameTot
    state = 1;
  % zweiter FIFO leer, erster voll ?
  elseif state == 1 && FIFO_Count1 == 0 && FIFO_Count0 == N_FB_TX_frame
    state = 0;
  elseif state == 1 && FIFO_Count1 == 0 && FIFO_Count0 == N_FB_TX_frameTot
    state = 0;
%  else
%    state = state;
  end
  
 sel = state;
end
