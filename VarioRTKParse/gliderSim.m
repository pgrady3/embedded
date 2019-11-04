clear;clc;close all;
%% Steup ASW 19

tfreq = 1000; %samples per sec
t = linspace(0, 10, tfreq * 10);
y0 = [0; 0; 0]; %x, z, z'
vx = 30.8; %60 knots
mass = 245 + 70;
g = 9.81;

varioWindow = 1.5 * tfreq;

%% Solve
dydt = @(t, y) [vx; y(3); liftForce(y, vx) / mass];
[t, y] = ode45(dydt, t, y0);

%% Analyze

x = y(:, 1);
z = y(:, 2);
vz = y(:, 3);
az = gradient(vz, t);
[windx, windz] = arrayfun(@(cx) wind(cx), x);

pitotv = vx + windx;
TEC = 0.5 * mass * pitotv.^2 + mass * g * z;
TECV = zeros(size(TEC));

for i = varioWindow + 1 : length(TECV)
   TECV(i) = (TEC(i) - TEC(i - varioWindow)) / (t(i) - t(i - varioWindow));
   TECV(i) = TECV(i) / (mass * g);
end

plot(t, vz); hold on; grid on;
plot(t, pitotv);
plot(t, TECV);
plot(t, windz);
legend('vz', 'pitot v', 'vario');