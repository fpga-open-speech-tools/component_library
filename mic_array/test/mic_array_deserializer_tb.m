% HdlCosimulation System Object creation (this Matlab function was created
% by the cosimWizard).
hdl = hdlcosim_mic_array_deserializer;

% Number of generated verification cases
Nverify = 5000;

% input word size/parameters
W_in = 24;  
F_in = 0;   
S_in = 1;  

% number of channels in each frame
num_channels = 16;

% frame start stimulus: indicates the beginning of each TDM frame. 
frame_start_stim = fi(zeros(1, Nverify*32), 0, 1,0);
frame_start_stim(1:(num_channels*32):end) = 1;

% output word size/parameters
W_out = 32;
F_out = 28;
S_out = 1;

% create random 24-bit input vectors representing the 24-bit words coming
% from the mic array
r = range(fi(0,S_in,W_in,F_in));  % create a fixed-point object with given parameters
in24_real = double(r(1)) + (double(r(2))-double(r(1))).*rand(Nverify,1);  % get array of input values for A (doubles)
in24_stim = fi(in24_real, S_in, W_in, F_in);

% the 24-bit words are MSB-aligned in a 32-bit slot, so shift the 24-bit
% words to the left by 8 bits.
in32_stim = bitsll(fi(in24_real, S_out, W_out, 0), 8);

% drive the input stimuli into the design
j=1;
for i=1:Nverify
    bits = in32_stim.bin(i,:);
    for bit=bits
        din = fi(str2num(bit), 0, 1, 0);
        frame_start = frame_start_stim(j);
        [dout, word_clock, channel, valid] = step(hdl, frame_start, din);
        j = j +1;
    end
    dout_history{i} = dout;
    channel_out_history{i} = channel;
    valid_out_history{i} = valid;
end


% the expected output is the 24 bit word represented as 32.28, normalized
% to +- 1 (i.e. all 24 bits are fractional); to do this, we represent the
% number as 32-bit signed (to get the sign extension), then shift left by 4
% bits center the 24-bit word in the 32-bits. 
expected_output = bitsll(fi(in24_real, S_out, W_out, 0), 4);
expected_output = reinterpretcast(expected_output, numerictype(S_out, W_out, F_out));

% Perform the desired comparison (with the latency between input
% and output appropriately corrected).
error_indices = [];
for i=1:Nverify
    if(expected_output(i) ~= dout_history{i})
        error_indices = [error_indices i];
    end 
end

if isempty(error_indices)
    disp('all tests passed')
else
    disp('errors occured at:')
    disp(error_indices)
end

% release the simulator
clear hdl;



