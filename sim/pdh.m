import Descriminator.RingResonator

% Conversions
DBM_TO_W  = @(DB) 10^(DB/10)/(1e3);
W_TO_DBM  = @(W)  10*log10(W) + 30;

% Parameters
N = 4;
BETA = 1.01;
F_MOD = 1e9;                     W_MOD = 2*pi*F_MOD;
LAMBDA_M = 1550e-9;
LASER_POWER_DBM = 13;
Pin = DBM_TO_W(LASER_POWER_DBM);
% Ring (construct BEFORE using its fields)
rr = RingResonator();

% Lock carrier to a ring resonance
m     = round((rr.SPEED_OF_LIGHT/LAMBDA_M)/rr.FSR_Hz);
F_RES = m*rr.FSR_Hz;             W_RES = 2*pi*F_RES;

% Sidebands
n   = (-N:N).';
Jn  = besselj(n, BETA);            
Wn  = W_RES + n*W_MOD;
Hn  = rr.Through_Ratio_omega(Wn);

PD_POW = Pin * sum(abs(Jn.*Hn).^2);

PD_POW_sum  = Pin * sum(abs(Jn.*Hn).^2);  
% Time-average PD power should match analytic sum of per-line powers
PD_POW_time = mean(PD_POW);

fprintf('Laser \n\t Laser Frequency: %f THz \n\t Laser Power mW: %f (%f dBm)\n',F_RES/1e12,DBM_TO_W(LASER_POWER_DBM)*1e3 ...
    ,LASER_POWER_DBM);
fprintf('Time-avg DC mW: %.6f (dBm %.2f)\n', PD_POW_time*1e3, W_TO_DBM(PD_POW_time));
fprintf('Sum-form DC  : %.6f mW  (%.2f dBm)\n', PD_POW_sum*1e3,  W_TO_DBM(PD_POW_sum));
fprintf('Relative error (time vs sum): %.3e\n', abs(PD_POW_time-PD_POW_sum)/max(PD_POW_sum,eps));

span = 5*rr.Freq_Linewidth_Hz;
df   = linspace(-span, span, 1501);             % Hz
Wc   = W_RES + 2*pi*df;                         % 1×M
Wn   = Wc + n*W_MOD;                            % (2N+1)×M
Hn   = rr.Through_Ratio_omega(Wn);
PD   = Pin * sum(abs(Jn.*Hn).^2, 1);            % 1×M

figure;
yyaxis left;  plot(df/1e6, PD*1e3, 'LineWidth',1.4); ylabel('PD DC (mW)');
yyaxis right; plot(df/1e6, 10*log10(PD)+30, '--', 'LineWidth',1.2); ylabel('PD DC (dBm)');
grid on; xlabel('\Delta f (MHz)');
title(sprintf('Through-port DC vs detune  (\\beta=%.2f, N=\\pm%d, span=\\pm%.2f linewidths)', ...
      BETA, N, span/rr.Freq_Linewidth_Hz));
legend('mW','dBm','Location','best');
