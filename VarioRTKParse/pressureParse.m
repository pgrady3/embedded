clear;clc;close all;

data = importdata('pressureLogCompare.txt');

alt1 = data(:, 1);
alt2 = data(:, 2);
press = data(:, 3);
velo = data(:, 4);
elapsed = (1:length(alt1)) / 20;

plot(elapsed, alt1); hold on;
plot(elapsed, alt2);
grid on;

% figure;
% yyaxis right
% plot(elapsed / 60, press); grid on;
% ylabel('Differential pressure (Pa)');
% xlabel('Time (min)');
% yyaxis left
% plot(elapsed / 60, alt1); hold on;
% plot(elapsed / 60, alt2); hold on;
% ylabel('Altitude (m)');