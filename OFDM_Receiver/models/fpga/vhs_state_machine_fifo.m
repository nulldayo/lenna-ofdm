function sel = vhs_state_machine_fifo(RST, FIFO_Empty, FIFO_Filled, Preamble_Det, DSPFree)

  persistent state, state = xl_state(1, {xlUnsigned,3,0});

  if RST
     state = 1; 
  else
     switch double(state)
         case 0, state = 1; % just in case...
         % IDLE state, wait for preamble, then switch to WRITE
         case 1, if Preamble_Det, state=2; end
         % WRITE state, wait for fifo fill, then switch to READ
         case 2, if FIFO_Filled, state=5; end
         % READ state, wait for fifo to empty
         case 3, if FIFO_Empty, state=1; end
         % Wait for DSP state (after filling FIFO, wait for available DSP)
         case 5, if DSPFree, state=3;end
         % defaults
         case 4, state=1;
         case 6, state=1;
         case 7, state=1;
     end
  end
  sel = state;
