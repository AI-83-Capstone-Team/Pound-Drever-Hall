%% Load CSV (auto-detects header)
data = readtable('data/85mA_1V_sin_800Hz_eom_nomod_1xattenuation.csv');


t    = data.TIME_ms;   % or data.("TIME ms") if MATLAB inserts space
in1  = data.IN1;
out1 = data.OUT1;

figure;
plot(t, in1, 'b-', 'LineWidth', 1.5); hold on;
plot(t, out1, 'r-', 'LineWidth', 1.5);
hold off;

xlabel('Time (ms)');
ylabel('Signal');
title('IN1 and OUT1 vs Time');
legend('IN1', 'OUT1');
grid on;
