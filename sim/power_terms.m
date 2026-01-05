import Descriminator.RingResonator

DBM_TO_W = @(dBm) 1e-3 * 10.^(dBm/10);
W_TO_DBM = @(W) 10*log10(W/1e-3);

% ---- User params ----
BETA = 1.09;
F_MOD = 500e6;  W_MOD = 2*pi*F_MOD;
LAMBDA_M = 1550e-9;
LASER_POWER_DBM = 13;

PIC_LOSS_dB = 20;

% Photodiode + Load 
R_PD = 1.0;   % A/W responsivity
RL   = 50;     % ohms (termination)

% Setup
Pin = DBM_TO_W(LASER_POWER_DBM);
Pin_PD = Pin * 10^(-PIC_LOSS_dB/10);

rr = RingResonator();

m     = round((rr.SPEED_OF_LIGHT/LAMBDA_M)/rr.FSR_Hz);
F_RES = m*rr.FSR_Hz;  W_RES = 2*pi*F_RES;

fprintf('Input power:     %.6f mW (%.2f dBm)\n', Pin*1e3,   W_TO_DBM(Pin));
fprintf('At PD (%.1f dB): %.6f mW (%.2f dBm)\n\n', PIC_LOSS_dB, Pin_PD*1e3, W_TO_DBM(Pin_PD));

% Detune Sweep
span = 5*rr.Freq_Linewidth_Hz;
df   = linspace(-span, span, 1501);          % Hz
Wc   = W_RES + 2*pi*df(:);                   % column vector (rad/s)

% Tones included
n = (-10:10);                                  % [-2 -1 0 1 2]
N = numel(n);

Jn = besselj(n, BETA);                       % 1xN

% Build omega for each tone: size = [Ndf x Ntones]
Wtones = Wc + (n .* W_MOD);                  % implicit expansion

% Ring response for each tone (reshape-safe even if function wants vector)
H = rr.Through_Ratio_omega(Wtones(:));
H = reshape(H, size(Wtones));                % [Ndf x N]

% Complex field amplitudes for each tone:
% A_n = sqrt(Pin_PD) * Jn * H(ω0+nΩ)
A = sqrt(Pin_PD) .* (H .* Jn);               % [Ndf x N]

% ---- DC powers (reuse A) ----
P_tone = abs(A).^2;                          % [Ndf x N]
PDC_df = sum(P_tone, 2);                     % [Ndf x 1]

% Index helpers for tones
idx_m2 = find(n==-2); idx_m1 = find(n==-1); idx_0 = find(n==0); idx_p1 = find(n==1); idx_p2 = find(n==2);

P0_df  = P_tone(:, idx_0);
Pm1_df = P_tone(:, idx_m1);
Pp1_df = P_tone(:, idx_p1);
Pm2_df = P_tone(:, idx_m2);
Pp2_df = P_tone(:, idx_p2);

% Print at resonance-ish (closest df to 0)
[~, i_res] = min(abs(df));
P0  = P0_df(i_res);  Pp1 = Pp1_df(i_res);  Pm1 = Pm1_df(i_res);  Pp2 = Pp2_df(i_res);  Pm2 = Pm2_df(i_res);
PDC = PDC_df(i_res);

fprintf('At detune ~0 (Δf=%.3f kHz):\n', df(i_res)/1e3);
fprintf('Carrier (n=0):   %.6f mW (%.2f dBm)\n', P0*1e3,  W_TO_DBM(P0));
fprintf('+1 sideband:     %.6f mW (%.2f dBm)\n', Pp1*1e3, W_TO_DBM(Pp1));
fprintf('-1 sideband:     %.6f mW (%.2f dBm)\n', Pm1*1e3, W_TO_DBM(Pm1));
fprintf('+2 sideband:     %.6f mW (%.2f dBm)\n', Pp2*1e3, W_TO_DBM(Pp2));
fprintf('-2 sideband:     %.6f mW (%.2f dBm)\n', Pm2*1e3, W_TO_DBM(Pm2));
fprintf('Total DC (sum):  %.6f mW (%.2f dBm)\n', PDC*1e3, W_TO_DBM(PDC));

% Worst-case (max over sweep)
[P0_worst,  i0]  = max(P0_df);
[Pp1_worst, ip1] = max(Pp1_df);
[Pm1_worst, im1] = max(Pm1_df);
[Pp2_worst, ip2] = max(Pp2_df);
[Pm2_worst, im2] = max(Pm2_df);
[PDC_worst, it]  = max(PDC_df);

fprintf('\nWorst-case over detune sweep (±%.2f linewidths):\n', span/rr.Freq_Linewidth_Hz);
fprintf('Carrier max:     %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', P0_worst*1e3,  W_TO_DBM(P0_worst),  df(i0)/1e6);
fprintf('+1 SB max:       %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', Pp1_worst*1e3, W_TO_DBM(Pp1_worst), df(ip1)/1e6);
fprintf('-1 SB max:       %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', Pm1_worst*1e3, W_TO_DBM(Pm1_worst), df(im1)/1e6);
fprintf('+2 SB max:       %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', Pp2_worst*1e3, W_TO_DBM(Pp2_worst), df(ip2)/1e6);
fprintf('-2 SB max:       %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', Pm2_worst*1e3, W_TO_DBM(Pm2_worst), df(im2)/1e6);
fprintf('Total DC max:    %.6f mW (%.2f dBm) at Δf = %.3f MHz\n', PDC_worst*1e3, W_TO_DBM(PDC_worst), df(it)/1e6);

% Fully-detuned bound (H=1)
J0 = besselj(0, BETA); J1 = besselj(1, BETA); J2 = besselj(2, BETA);
P0_far   = Pin_PD * abs(J0)^2;
Psb1_far = Pin_PD * abs(J1)^2;
Psb2_far = Pin_PD * abs(J2)^2;
PDC_far  = P0_far + 2*Psb1_far + 2*Psb2_far;

fprintf('\nFully-detuned (H≈1) bound:\n');
fprintf('Carrier:         %.6f mW (%.2f dBm)\n', P0_far*1e3,   W_TO_DBM(P0_far));
fprintf('Each ±1 SB:      %.6f mW (%.2f dBm)\n', Psb1_far*1e3, W_TO_DBM(Psb1_far));
fprintf('Each ±2 SB:      %.6f mW (%.2f dBm)\n', Psb2_far*1e3, W_TO_DBM(Psb2_far));
fprintf('Total DC:        %.6f mW (%.2f dBm)\n', PDC_far*1e3,  W_TO_DBM(PDC_far));

% ---- AC at f_mod and 2*f_mod (reuse A) ----
% Power envelope harmonic coefficients:
% C1 = sum_n A_n * conj(A_{n-1})  (adjacent pairs)  -> @ f_mod
% C2 = sum_n A_n * conj(A_{n-2})  (skip-1 pairs)    -> @ 2f_mod
C1 = sum( A(:,2:end) .* conj(A(:,1:end-1)), 2 );   % [Ndf x 1]
C2 = sum( A(:,3:end) .* conj(A(:,1:end-2)), 2 );   % [Ndf x 1]

% Optical AC amplitude in power envelope:
P1_opt_pk  = 2*abs(C1);
P2_opt_pk  = 2*abs(C2);
P1_opt_rms = P1_opt_pk / sqrt(2);
P2_opt_rms = P2_opt_pk / sqrt(2);

% Detected current and RF power into RL:
I1_rms = R_PD .* P1_opt_rms;
I2_rms = R_PD .* P2_opt_rms;
PRF1   = (I1_rms.^2) .* RL;
PRF2   = (I2_rms.^2) .* RL;

% ---- Plots ----
df_MHz = df/1e6;

figure; grid on; hold on;
plot(df_MHz, P0_df*1e3,  'LineWidth',1.4);
plot(df_MHz, Pp1_df*1e3, 'LineWidth',1.4);
plot(df_MHz, Pm1_df*1e3, 'LineWidth',1.4);
plot(df_MHz, Pp2_df*1e3, 'LineWidth',1.4);
plot(df_MHz, Pm2_df*1e3, 'LineWidth',1.4);
plot(df_MHz, PDC_df*1e3, '--', 'LineWidth',1.2);
xlabel('\Delta f (MHz)'); ylabel('Power (mW)');
title(sprintf('Through-port powers vs detune (\\beta=%.2f, n=0,\\pm1,\\pm2, PIC loss=%.0f dB)', BETA, PIC_LOSS_dB));
legend('Carrier','+1','-1','+2','-2','Total DC','Location','best');
hold off;

figure; grid on; hold on;
plot(df_MHz, P1_opt_pk*1e3, 'LineWidth', 1.4);
plot(df_MHz, P2_opt_pk*1e3, 'LineWidth', 1.4);
xlabel('\Delta f (MHz)'); ylabel('Optical AC power (mW)');
title(sprintf('Optical AC envelope vs detune (\\beta=%.2f, f_{mod}=%.0f MHz)', BETA, F_MOD/1e6));
legend('@ f_{mod}', '@ 2f_{mod}', 'Location', 'best');
hold off;

figure; grid on; hold on;
plot(df_MHz, W_TO_DBM(PRF1), 'LineWidth', 1.4);
plot(df_MHz, W_TO_DBM(PRF2), 'LineWidth', 1.4);
xlabel('\Delta f (MHz)'); ylabel('RF power into R_L (dBm)');
title(sprintf('Detected RF vs detune (R=%.2f A/W, R_L=%.0f \\Omega)', R_PD, RL));
legend('@ f_{mod}', '@ 2f_{mod}', 'Location', 'best');
hold off;
