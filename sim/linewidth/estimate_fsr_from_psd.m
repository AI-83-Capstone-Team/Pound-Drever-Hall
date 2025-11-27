% Plot the frequency-noise PSD from the output csv from process_spy.py
function [T,f_Hz,psd_dB] = estimate_fsr_from_psd(psd_csv)
try
    tb = readtable(psd_csv);
catch ME
    fprintf('Failed to read table from: %s. Ensure that path is correct!\n', psd_csv);
    rethrow(ME);
end

f_Hz   = tb{:, 1}*1e6;   % frequency [Hz]
psd_dB = tb{:, 2};

% start from a bit past dc
mask = f_Hz > 5e6;
f2   = f_Hz(mask);
y2   = psd_dB(mask);

% average the data to make it easier to detect the fsr drops
win   = 20;
y2_s  = filter(ones(1,win)/win, 1, y2);

% minima as peaks on -y2_s
[pks, locs] = findpeaks(-y2_s, f2, 'MINPEAKDISTANCE', 5e6);

% keep only deeper minima
thr      = mean(pks);
keep     = pks > thr;
locs_use = locs(keep);

% estimate period between minima
spacing_Hz = diff(locs_use);
fsr_Hz     = median(spacing_Hz);
fsr_MHz    = fsr_Hz/1e6;
T = (1/fsr_Hz);
fprintf('Estimated frequency f = %.2f MHz\n', fsr_MHz);
fprintf('Estimate  period T =  %f ms\n',(1/fsr_Hz)*1000);
end