clear; close all; clc;

%data = importdata('driveWithAni.TXT');
%data = importdata('accTest.TXT');
%data = importdata('compareYard.TXT');
%data = importdata('compareFixed2.TXT');
%data = importdata('driveRTKAni.TXT');
%data = importdata('EFXPowerPlant.TXT');
%data = data(71451:147000, :);

data = importdata('EFXBlueFloat.TXT');
data = data(5000:end, :);
%data = data(27800:36400, :); %long thermal 1. Poor centering. South side good
%data = data(44800:47300, :); %Strong thermal. East side good
data = data(13100:21200, :); %Thermal with mitch. West side good

lat = data(:, 1) ./ 1e7;
lon = data(:, 2) ./ 1e7;
gpsAlt = data(:, 3);
gpsFix = data(:, 4);
gpsComp = data(:, 5);
vN = data(:, 6);
vE = data(:, 7);
vD = data(:, 8);
gpsVAcc = data(:, 10) / 1000;
gpsHAcc = data(:, 11) / 1000;
baroAlt = data(:, 12);
baroV = data(:, 13);
ax = data(:, 14);
ay = data(:, 15);
az = data(:, 16);
roll = data(:, 17);
pitch = data(:, 18);
yaw = data(:, 19);
elapsed = data(:, 20) / 1000;
accel = [ax ay az];
vtot = [vN vE vD];
vtot = sqrt(sum(vtot.^2, 2));

ke = 0.5 * vtot.^2;
pe = 9.801 * gpsAlt;
te = ke + pe;

uncomp = gradient(gpsAlt) ./ gradient(elapsed);
uncomp = smooth(uncomp, 21);

TEC = gradient(te);
TEC = smooth(TEC, 21);

baroV = smooth(baroV, 21);

angle = data(:, 17:19) ./ (180 / pi);
%angle = [0 pi/4 0]
quat = eul2quat(angle, 'XYZ');
r = [0 0 1]; %N E D
vec = quatrotate(quat, r);

accelDown = sum(vec .* accel, 2);

accVz = cumsum(az - 1);
accVzFilt = highpass(accVz, 1/100, 10);
accDown = cumsum(accelDown - 1);
%accDownFilt = highpass(accDown, 1/100, 10);
accDownFit = polyfit(elapsed, accDown, 1);
accDownFilt = accDown - polyval(accDownFit, elapsed);

accVzFilt = smooth(accVzFilt, 11);
vZ = smooth(-vD, 1);
baroV = smooth(baroV, 1);

[N, E, D] = geodetic2ned(lat, lon, gpsAlt, lat(1), lon(1), 0, referenceEllipsoid('GRS80','m'));

% figure;
% plot(x, y); axis square;

figure;
yyaxis left;
plot(baroAlt); hold on;
plot(gpsAlt);
legend('Baro alt', 'GPS alt');
yyaxis right;
plot(gpsFix);

figure;
ax1 = subplot(2, 1, 1);

%plot(elapsed, vZ); hold on;
plot(elapsed, TEC); hold on;
plot(elapsed, uncomp);
plot(elapsed, baroV); hold on;
%plot(elapsed, TEC);
legend('gps TEC', 'gps uncomp', 'baro uncomp');  grid on;
%plot(elapsed, accDownFilt);
%plot(elapsed, accDown);
ylabel('Vario in m/s')
ylim([-3 3])

ax2 = subplot(2, 1, 2);
plot(elapsed, vtot); grid on;
xlabel('Time in s')
ylabel('3D V in m/s')
%ylim([15 40])

linkaxes([ax1 ax2], 'x')

TECclip = max(min(TEC, 3), -3);
%TECclip = max(min(uncomp, 3), -3);

figure;
scatter3(E, N, -D, 3, TECclip);
xlabel('East (m)'); ylabel('North (m)'); zlabel('Up (m)');
axis equal; colorbar; title('Position vs Vario in m/s');

vclip = max(min(vtot, 28), 22);

figure;
scatter3(E, N, -D, 3, vclip);
xlabel('East'); ylabel('North'); zlabel('Up');
axis equal; colorbar; title('Position vs speed');

figure;
scatter3(E, N, (1:length(E))', 3);
title('Position vs data point');

figure;
plot(gpsAlt); grid on;
title('Point vs alt');