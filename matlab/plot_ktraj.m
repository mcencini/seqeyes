function plot_ktraj(folderPath, ktraj, ktraj_adc)
%PLOT_KTRAJ Visualize/exported vs MATLAB k-space trajectories for SeqEyes.
%   PLOT_KTRAJ() opens a folder selection dialog and loads ktraj.txt and
%   ktraj_adc.txt from that directory.
%   PLOT_KTRAJ(FOLDERPATH) reads the files directly from FOLDERPATH.
%   PLOT_KTRAJ(FOLDERPATH, KTRAJ, KTRAJ_ADC) overlays MATLAB-computed
%   trajectories KTRAJ (Nx3) and KTRAJ_ADC (Mx3) with exported ones for comparison.
%
%   Each txt file must contain N rows and 3 columns [k_x k_y k_z] in 1/m.
%
%   Figure 1: 3D overlay of exported and MATLAB trajectories.
%   Figure 2: Per-axis comparison (kx, ky, kz) with error stats.

    if nargin < 1
        folderPath = [];
    end
    if nargin < 2
        ktraj = [];
    end
    if nargin < 3
        ktraj_adc = [];
    end

    % Resolve folder path (exported txt location)
    if isempty(folderPath)
        folderPath = uigetdir(pwd, 'Select folder containing ktraj*.txt');
        if folderPath == 0
            fprintf('No folder selected. Aborting.\n');
            return;
        end
    end

    trajPath = fullfile(folderPath, 'ktraj.txt');
    trajAdcPath = fullfile(folderPath, 'ktraj_adc.txt');

    if ~isfile(trajPath)
        error('Cannot find %s', trajPath);
    end
    if ~isfile(trajAdcPath)
        error('Cannot find %s', trajAdcPath);
    end

    traj = readTrajectory(trajPath);
    trajAdc = readTrajectory(trajAdcPath);
    % Validate optional MATLAB arrays
    if ~isempty(ktraj)
        ktraj = validateTraj(ktraj, 'ktraj');        % standardize to Nx3
    end
    if ~isempty(ktraj_adc)
        ktraj_adc = validateTraj(ktraj_adc, 'ktraj_adc');  % standardize to Nx3
    end

    % Figure 1: Continuous trajectory (exported vs MATLAB)
    figure('Name', 'SeqEyes K-space Trajectory (continuous)', 'Color', 'w');
    hold on;
    ax = gca; set(ax, 'SortMethod', 'childorder'); % later drawn objects on top
    % Exported continuous: blue line
    plot3(traj(:,1), traj(:,2), traj(:,3), '-o', 'Color', [0 0.447 0.741], ...
        'LineWidth', 1.0, 'DisplayName', 'ktraj (exported)');
    % MATLAB continuous: green dots
    if ~isempty(ktraj)
        hold on;
        plot3(ktraj(:,1), ktraj(:,2), ktraj(:,3), ...
            'LineStyle','none','Marker','.', 'MarkerSize',18, ...
            'Color', [0.1 0.6 0.2], 'DisplayName', 'ktraj (MATLAB)');
    end
    xlabel('k_x (1/m)');
    ylabel('k_y (1/m)');
    zlabel('k_z (1/m)');
    grid on; axis equal;
    legend('Location', 'best');
    title('Continuous k-space: exported vs MATLAB (dots)');
    hold off;

    % Figure 2: ADC sampling trajectory（exported vs MATLAB）— different dot styles
    figure('Name', 'SeqEyes K-space Trajectory (ADC)', 'Color', 'w');
    hold on;
    ax = gca; set(ax, 'SortMethod', 'childorder'); % later drawn objects on top
    % Exported ADC: red squares (filled)
    scatter3(trajAdc(:,1), trajAdc(:,2), trajAdc(:,3), 12, 'Marker', 's', ...
        'MarkerFaceColor', [0.9 0.1 0.1], 'MarkerEdgeColor', 'none', ...
        'DisplayName', 'ktraj\_adc (exported)');
    % MATLAB ADC: purple triangles (open)
    if ~isempty(ktraj_adc)
        scatter3(ktraj_adc(:,1), ktraj_adc(:,2), ktraj_adc(:,3), 20, 'Marker', '^', ...
            'MarkerEdgeColor', [0.5 0 0.8], 'MarkerFaceColor', 'none', ...
            'LineWidth', 0.7, 'DisplayName', 'ktraj\_adc (MATLAB)');
    end
    xlabel('k_x (1/m)');
    ylabel('k_y (1/m)');
    zlabel('k_z (1/m)');
    grid on; axis equal;
    legend('Location', 'best');
    title('ADC k-space samples: exported vs MATLAB (dots)');
    hold off;

    % Figure 3: kx vs sample index (exported vs MATLAB)
    figure('Name', 'SeqEyes Kx vs sample (overlay)', 'Color', 'w');
    hold on;
    colorsExp = [0 0.447 0.741];
    colorsM   = [0.2 0.7 0.2];
    % Exported kx
    nx = size(traj,1);
    plot(1:nx, traj(:,1), '-o', 'Color', colorsExp, 'LineWidth', 1.0, ...
        'MarkerSize', 3, 'DisplayName', 'kx (exported)');
    % MATLAB kx (if provided), aligned by shortest length
    if ~isempty(ktraj)
        n = min(nx, size(ktraj,1));
        plot(1:n, ktraj(1:n,1), 'LineStyle', 'none', 'Marker', '.', ...
            'Color', colorsM, 'MarkerSize', 14, 'DisplayName', 'kx (MATLAB)');
    end
    grid on;
    xlabel('sample index'); ylabel('k_x (1/m)');
    legend('Location', 'best');
    title('k_x vs sample: exported (line) vs MATLAB (dots)');
    hold off;

    % Per-axis comparison if MATLAB arrays provided
    if ~isempty(ktraj)
        figure('Name', 'SeqEyes K-space Trajectory (per-axis compare)', 'Color', 'w');
        tiledlayout(3,1);
        labels = {'k_x (1/m)', 'k_y (1/m)', 'k_z (1/m)'};
        colorsExp = [0 0.447 0.741];
        colorsM   = [0.2 0.7 0.2];
        % Align lengths for line comparison (no resampling, min length)
        n = min(size(traj,1), size(ktraj,1));
        Texp = traj(1:n, :);
        Tmat = ktraj(1:n, :);
        % Compute RMSE per axis
        rmse = sqrt(mean((Texp - Tmat).^2, 1, 'omitnan'));
        for r = 1:3
            nexttile;
            plot(1:n, Texp(:,r), '-', 'Color', colorsExp, 'LineWidth', 1.0, 'DisplayName', 'exported');
            hold on;
            plot(1:n, Tmat(:,r), '-', 'Color', colorsM, 'LineWidth', 1.0, 'DisplayName', 'MATLAB');
            grid on;
            xlabel('sample index');
            ylabel(labels{r});
            title(sprintf('%s | RMSE=%.3g', labels{r}, rmse(r)));
            legend('Location', 'best');
            hold off;
        end
        % Print summary to console
        fprintf('RMSE (exported vs MATLAB): kx=%.6g, ky=%.6g, kz=%.6g (1/m)\n', rmse(1), rmse(2), rmse(3));
    end
    
    % Figure 3: Difference (exported - MATLAB), continuous and ADC (if provided)
    if ~isempty(ktraj)
        % Continuous diff
        n = min(size(traj,1), size(ktraj,1));
        Dcont = traj(1:n, :) - ktraj(1:n, :);
        figure('Name', 'SeqEyes K-space Difference (exported - MATLAB)', 'Color', 'w');
        tiledlayout(3,1);
        diffLabels = {'\Deltak_x (1/m)', '\Deltak_y (1/m)', '\Deltak_z (1/m)'};
        for r = 1:3
            nexttile;
            plot(1:n, Dcont(:,r), '-', 'Color', [0.1 0.1 0.1], 'LineWidth', 1.0, 'DisplayName', 'cont diff');
            hold on; grid on;
            xlabel('sample index'); ylabel(diffLabels{r});
            % If MATLAB ADC provided, overlay ADC diff as points
            if ~isempty(ktraj_adc)
                m = min(size(trajAdc,1), size(ktraj_adc,1));
                Dadc = trajAdc(1:m, :) - ktraj_adc(1:m, :);
                scatter(1:m, Dadc(:,r), 8, 'filled', 'MarkerFaceColor', [0.85 0.2 0.2], ...
                    'MarkerFaceAlpha', 0.8, 'DisplayName', 'adc diff');
            end
            yline(0, '--', 'Color', [0.6 0.6 0.6], 'LineWidth', 0.8);
            legend('Location', 'best');
            hold off;
        end
        % Print max|diff| summary
        mx = max(abs(Dcont), [], 1, 'omitnan');
        fprintf('Max |diff| (exported - MATLAB): kx=%.6g, ky=%.6g, kz=%.6g (1/m)\n', mx(1), mx(2), mx(3));
        if ~isempty(ktraj_adc)
            m = min(size(trajAdc,1), size(ktraj_adc,1));
            Dadc = trajAdc(1:m, :) - ktraj_adc(1:m, :);
            mxa = max(abs(Dadc), [], 1, 'omitnan');
            fprintf('Max |diff| ADC (exported - MATLAB): kx=%.6g, ky=%.6g, kz=%.6g (1/m)\n', mxa(1), mxa(2), mxa(3));
        end
    end
end

function data = readTrajectory(path)
    data = readmatrix(path, 'FileType', 'text');
    if size(data, 2) ~= 3
        error('Expected 3 columns in %s but found %d.', path, size(data, 2));
    end
end

function arr = validateTraj(arr, name)
% Validate and standardize trajectory arrays to Nx3 [kx ky kz] (1/m).
% Accepts Nx3 or 3xN numeric; returns Nx3.
    if ~isnumeric(arr)
        error('Argument %s must be numeric.', name);
    end
    [r, c] = size(arr);
    if c == 3
        % Already Nx3
        return;
    elseif r == 3
        % 3xN -> transpose to Nx3
        arr = arr.';
        return;
    else
        error('Argument %s must be size Nx3 or 3xN; got %dx%d.', name, r, c);
    end
end
