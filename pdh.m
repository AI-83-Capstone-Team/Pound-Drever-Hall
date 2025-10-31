import Descriminator.RingResonator

% Conversions
DBM_TO_W  = @(DB) 10^(DB/10)/(1e3);
W_TO_DBM  = @(W)  10*log10(W) + 30;

% Parameters
N = 4;
BETA = 1.01;
F_MOD = 1e9;                     W_MOD = 2*pi*F_MOD;
LAMBDA_M = 1550e-9;
LASER_POWER_DBM = 5;
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

% t
t = linspace(0, 1e-3, 200000);     

% Field vs time (vectorized)
E_REF = sqrt(Pin) * ( (Jn.*Hn).' * exp(1j*Wn.*t) );

% Time-average PD power should match analytic sum of per-line powers
PD_POW_time = mean(abs(E_REF).^2);

fprintf('Laser \n\t Laser Frequency: %f THz \n\t Laser Power mW: %f (%f dBm)\n',F_RES/1e12,DBM_TO_W(LASER_POWER_DBM)*1e3 ...
    ,LASER_POWER_DBM);
fprintf('Time-avg DC mW: %.6f (dBm %.2f)\n', PD_POW_time*1e3, W_TO_DBM(PD_POW_time));


% Frequency Sweep For DC Power
span_Hz   = rr.Freq_Linewidth_Hz;
detune_Hz = linspace(-10*span_Hz/2, 10*span_Hz/2, 1000);   
M         = numel(detune_Hz);


t = linspace(0, 10e-6, 100000);  

PD_vec = zeros(1, M);

for k = 1:M
    Wc = W_RES + 2*pi*detune_Hz(k);
    Wn = Wc + n*W_MOD;


    Hn = rr.Through_Ratio_omega(Wn);

    % E_REF(t) = sqrt(Pin) * sum_n (Jn .* Hn) .* exp(1j * Wn * t)
    E_REF = sqrt(Pin) * sum( (Jn.*Hn) .* exp(1j * (Wn .* t)), 1 );

    % PD DC power via time average
    PD_vec(k) = mean(abs(E_REF).^2);
end


figure; plot(detune_Hz/1e6, PD_vec*1e3, 'LineWidth', 1.5); grid on;
xlabel('\Delta f (MHz)');
ylabel('PD DC Power (mW)');
title(sprintf('Through-port DC vs detune (\\beta=%.2f, N=\\pm%d, span=0.5 linewidth = %.3f MHz)', ...
      BETA, N, span_Hz/1e6));
