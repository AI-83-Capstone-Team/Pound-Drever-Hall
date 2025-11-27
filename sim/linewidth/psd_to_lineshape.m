function psd_to_lineshape(varargin)
    % psd_to_lineshape(csv1, csv2, ...)
    % psd_to_lineshape({csv1, csv2, ...})
    %
    % For each PSD CSV:
    %   - calls estimate_fsr(csv_path) to get T, f_Hz, psd_dB
    %   - converts to S_nu(f)
    %   - plots all S_nu(f) on one loglog figure
    %   - computes lineshape and plots all spectra on one figure
    %
    % No return values, just figures.
    % Laser Power for all runs

    if nargin == 0
        error('Provide at least one PSD CSV file.');
    end

    if nargin == 1 && iscell(varargin{1})
        files = varargin{1};
    else
        files = varargin;
    end

    c      = 3e8;
    lambda = 1550e-9;
    f0     = c / lambda;

    fig_csvpsd = figure; hold on;
    fig_psd    = figure; hold on;
    fig_ls     = figure; hold on;

    labels = cell(size(files));
    for k = 1:numel(files)
        [~, name, ext] = fileparts(files{k});
        labels{k} = [name ext];
    end
    
    for k = 1:numel(files)
        csv_path = files{k};

        [T, f_Hz, psd_dB] = estimate_fsr_from_psd(csv_path);
        psd_dB = psd_dB(1:end);
        f_Hz   = f_Hz(1:end);

        figure(fig_csvpsd);
        semilogx(f_Hz, psd_dB);

        df   = mean(diff(f_Hz));
        P_W  = 1e-3 * 10.^(psd_dB/10);
        S_I  = P_W / df;

        K0   = (pi*T)^2 * sin(2*pi*f0*T).^2;
        Kf   = (sinc(f_Hz*T)).^2;

        eps      = 1e-2;
        valid    = Kf > eps;

        S_nu        = nan(size(S_I));
        S_nu(valid) = S_I(valid) ./ (K0 * Kf(valid));

        figure(fig_psd);
        loglog(f_Hz(valid), S_nu(valid));
        
        f_min = min(f_Hz(valid));

        fnpsd = @(x) interp1( ...
                        f_Hz(valid), S_nu(valid), ...
                        x, 'linear', 0) .* (x >= f_min);
        
        fs   = 1/T;
        E_0  = sqrt(5e-3);
        v_0  = 0;
        t_0  = Inf;

        [f, spectrum] = lineshape(fnpsd, fs, E_0, v_0, t_0);

        t0_lw = 1/f_min;
        [FWHM,~] = approx_linewidth(fnpsd, t0_lw);
        fprintf('%s: Estimated FWHM: %.8f kHz\n', labels{k}, FWHM/1000);

        figure(fig_ls);
        plot(f, spectrum);
    end

    figure(fig_csvpsd);
    xlabel('Frequency (Hz)');
    ylabel('PSD (dBm/bin)');
    title('Measured PSD from CSV (overlay)');
    grid on;
    legend(labels, 'Interpreter', 'none', 'Location', 'best');

    figure(fig_psd);
    xlabel('Frequency (Hz)');
    ylabel('S_\nu(f) (Hz^2/Hz)');
    title('Recovered Frequency-Noise PSD (overlay)');
    grid on;
    legend(labels, 'Interpreter', 'none', 'Location', 'best');
    
    figure(fig_ls);
    xlabel('Frequency (Hz)');
    ylabel('Normalized spectrum');
    title('Laser Lineshape from Frequency-Noise PSD (overlay)');
    grid on;
    legend(labels, 'Interpreter', 'none', 'Location', 'best');

function [f, spectrum] = lineshape(fnpsd, fs, E_0, v_0, t_0)
    n = 2000;
    n = 2^nextpow2(n);

    ts = 1/fs;

    acf = zeros(1,n);

    for t = 0:n-1
        tau = t*ts;

        func      = @(x) sin(pi.*x.*tau).^2./x.^2;
        integrand = @(x) func(x).*fnpsd(x);
        integ     = integral(integrand, 1/t_0, inf);

        acf(1,t+1) = E_0^2 * exp(1i*2*pi*v_0*tau) * exp(-2*integ);
    end

    y        = cat(2, flip(acf), acf);
    absfft   = abs(fftshift(fft(y)));
    spectrum = transpose(absfft / max(absfft));
    f        = transpose(linspace(-fs/2, fs/2, 2*n-1));
    spectrum = spectrum(2:2*n); 
end

function [FWHM, fm] = approx_linewidth(fnpsd, t0)
    options = optimset('Display','off');
    func = @(x) fnpsd(x) - 8.*log(2).*x/pi^2;
    fm = fsolve(func,10^(-12),options);
    
    integrand = @(x) fnpsd(x).*heaviside(func(x));
    A = integral(integrand,1/t0,inf);
    FWHM = sqrt(8*log(2)*A);
end

end
