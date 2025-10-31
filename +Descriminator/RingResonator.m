classdef RingResonator
    properties
        % constants / indices
        SPEED_OF_LIGHT = 3e8
        n_g   = 3.76
        n_eff = 2.837

        % geometry
        L     = 0.01252849555921
        lambda0 = 1550e-9

        % loss (POWER) in dB/m
        alpha_wg_dB = 40
        alpha_p_Np_per_m
        a

        % coupler-1 (single-bus all-pass; your k2=0 => t2=1)
        k = NaN
        t = NaN

        % derived parameters
        FSR_Hz
        FWHM_lambda
        Freq_Linewidth_Hz
        Finesse
        Q_i
        Q1
        Q2
        Q_loaded
    end

    methods
        function obj = RingResonator(varargin)
            p = inputParser;
            addParameter(p, 'L', []);
            addParameter(p, 'alpha_wg_dB', []);
            addParameter(p, 'n_eff', []);
            addParameter(p, 'n_g', []);
            addParameter(p, 'lambda0', []);
            addParameter(p, 'k', []);
            addParameter(p, 'use_critical', true);
            parse(p, varargin{:});
            S = p.Results;

            if ~isempty(S.L),             obj.L = S.L; end
            if ~isempty(S.alpha_wg_dB),   obj.alpha_wg_dB = S.alpha_wg_dB; end
            if ~isempty(S.n_eff),         obj.n_eff = S.n_eff; end
            if ~isempty(S.n_g),           obj.n_g = S.n_g; end
            if ~isempty(S.lambda0),       obj.lambda0 = S.lambda0; end

            % dB/m (power) -> Np/m (power)
            NP_PER_DB_POWER       = log(10)/10;
            obj.alpha_p_Np_per_m  = NP_PER_DB_POWER * obj.alpha_wg_dB;

            % Field round-trip (A = e^{-α_p L}, a = sqrt(A))
            obj.a = exp(-obj.alpha_p_Np_per_m * obj.L / 2);

            % FSR using n_eff (matches your script)
            obj.FSR_Hz = obj.SPEED_OF_LIGHT / (obj.n_eff * obj.L);

            % Coupling
            if S.use_critical && isempty(S.k)
                obj.k = sqrt(1 - obj.a^2);           
            elseif ~isempty(S.k)
                obj.k = S.k;
            else
                obj.k = 0.35;
            end

            obj.t = sqrt(1 - obj.k^2);

            obj.FWHM_lambda       = (obj.lambda0^2) * (1 - obj.a*obj.t) / (obj.n_g * obj.L * pi * sqrt(obj.a*obj.t));
            obj.Freq_Linewidth_Hz = (obj.SPEED_OF_LIGHT / obj.lambda0^2) * obj.FWHM_lambda;
            obj.Finesse           = obj.FSR_Hz / obj.Freq_Linewidth_Hz;

            f0         = obj.SPEED_OF_LIGHT / obj.lambda0;
            obj.Q_i    = (2*pi * obj.n_g)              / (obj.alpha_p_Np_per_m * obj.lambda0);     % intrinsic
            obj.Q1     = (2*pi * obj.n_g * obj.L)      / (obj.lambda0 * obj.k^2);                  % coupler-1
            obj.Q2     = Inf;  % single-bus default (set via method if you want k2>0)
            Ql_from_bw = f0 / obj.Freq_Linewidth_Hz;
            obj.Q_loaded = Ql_from_bw;  
        end

        % ---------- Field transfer ----------
        function E = Through_Ratio_omega(obj, omega)
            betaL = (obj.n_eff .* omega ./ obj.SPEED_OF_LIGHT) .* obj.L;
            ejphi = exp(-1j * betaL);
            num   = (obj.t - obj.a .* ejphi);
            den   = (1     - obj.a .* obj.t .* ejphi);
            E     = num ./ den;
        end

        function E = Through_Ratio_lambda(obj, lambda)
            f     = obj.SPEED_OF_LIGHT ./ lambda;
            omega = 2*pi*f;
            E     = obj.Through_Ratio_omega(omega);
        end

        % ---------- Q helper to mirror [Qi,Q1,Q2] ----------
        function [Qi,Q1,Q2,Q_loaded] = Q_factors(obj, varargin)
            % Optional: 'k2', value (default 0). Only affects Q2.
            p = inputParser; p.addParameter('k2', 0); p.parse(varargin{:});
            k2 = p.Results.k2;
            if k2 > 0
                Q2 = (2*pi * obj.n_g * obj.L) / (obj.lambda0 * k2^2);
            else
                Q2 = Inf;
            end
            Qi = obj.Q_i;
            Q1 = obj.Q1;
            Q_loaded = 1 / (1/Qi + 1/Q1 + 1/Q2);
        end

        % Plotting
        function PlotTransmission(obj, varargin)
            p = inputParser; p.addParameter('fsr_span', 10); p.parse(varargin{:});
            fsr_span = p.Results.fsr_span;

            detune_Hz = linspace(-fsr_span*obj.FSR_Hz, fsr_span*obj.FSR_Hz, 100000);
            omega     = 2*pi*detune_Hz;
            Tpower    = abs(obj.Through_Ratio_omega(omega)).^2;

            figure;
            plot(detune_Hz/1e9, Tpower, 'LineWidth', 1.5); grid on;
            xlabel('\Delta f (GHz)');
            ylabel('Power transmission  |E_{out}/E_{in}|^2');
            title(sprintf('Ring Through-Port vs Detuning (FSR = %.4f GHz)', obj.FSR_Hz/1e9));
            legend('Through-port power', 'Location', 'best')
        end

        function PlotFieldPhase(obj, varargin)
            p = inputParser; p.addParameter('fsr_span', 10); p.parse(varargin{:});
            fsr_span = p.Results.fsr_span;

            detune_Hz = linspace(-fsr_span*obj.FSR_Hz, fsr_span*obj.FSR_Hz, 100000);
            omega     = 2*pi*detune_Hz;
            E         = obj.Through_Ratio_omega(omega);

            figure;
            plot(detune_Hz/1e9, unwrap(angle(E))*180/pi, 'LineWidth', 1.5); grid on;
            xlabel('\Delta f (GHz)');
            ylabel('Phase (deg)');
            title(sprintf('Through-Port Phase vs Detuning (FSR = %.4f GHz)', obj.FSR_Hz/1e9));
            legend('Through-port phase', 'Location', 'best')
        end

        function PlotFieldAndPhase(obj, varargin)
            p = inputParser; p.addParameter('fsr_span', 2.5); p.parse(varargin{:});
            fsr_span = p.Results.fsr_span;

            detune_Hz = linspace(-fsr_span*obj.FSR_Hz, fsr_span*obj.FSR_Hz, 200001);
            omega     = 2*pi*detune_Hz;

            E     = obj.Through_Ratio_omega(omega);
            mag   = abs(E);
            phdeg = angle(E)*180/pi;
            phdeg = mod(phdeg + 180, 360) - 180;
            phdeg(mag < 1e-8) = NaN;

            figure; grid on; hold on;
            yyaxis left
            plot(detune_Hz/1e9, 10*log10(mag.^2), 'LineWidth', 1.5);
            ylabel('Through power (dB)');

            yyaxis right
            plot(detune_Hz/1e9, phdeg, '--', 'LineWidth', 1.25);
            ylabel('Phase (deg)');

            xlabel('\Delta f (GHz)');
            title(sprintf('Through Field & Phase vs Detuning (FSR = %.4f GHz)', obj.FSR_Hz/1e9));
            legend({'Power','Phase'}, 'Location','best');
        end
    end
end
