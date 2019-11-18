clear; clc; close all;

data = importdata('EFXBlueFloat.TXT');
data = data(5000:end, :);
%data = data(27800:36400, :); %long thermal 1. Poor centering. South side good
%data = data(44800:47300, :); %Strong thermal. East side good
data = data(13100:21200, :); %Thermal with mitch. West side good

lat = data(:, 1) ./ 1e7;
lon = data(:, 2) ./ 1e7;
gpsAlt = data(:, 3);
gpsFix = data(:, 4);
vN = data(:, 6);
vE = data(:, 7);
vD = data(:, 8);
elapsed = data(:, 20) / 1000;

vtot = zeros(size(lat));
%vtotWind = zeros(size(lat));
varioComp = zeros(size(lat));
isCircling = zeros(size(lat));
heading = zeros(size(lat));
windE = zeros(size(lat));
windN = zeros(size(lat));
%ke = zeros(size(lat));
%pe = zeros(size(lat));
%te = zeros(size(lat));
%keWind = zeros(size(lat));
%teWind = zeros(size(lat));
varioCompWind = zeros(size(lat));
[N, E, D] = geodetic2ned(lat, lon, gpsAlt, lat(1), lon(1), 0, referenceEllipsoid('GRS80','m'));

VARIO_WINDOW = 10;
WIN1 = 40;
WIN2 = 80;
HEADING_WIN = 10;
WIND_DAMP = 16;

WIND_BINS = [45; 135; 225; 315];
binIdx = ones(size(WIND_BINS));

figure(1); clf; scatter3(E, N, -D, 1); grid on; hold on; 

for i = 500:length(lat)
    vNow = [vN(i) vE(i) vD(i)];
    vOld = [vN(i-VARIO_WINDOW) vE(i-VARIO_WINDOW) vD(i-VARIO_WINDOW)];
    vNowWind = [vN(i)-windN(i-1) vE(i)-windE(i-1) vD(i)];
    vOldWind = [vN(i-VARIO_WINDOW)-windN(i-1) vE(i-VARIO_WINDOW)-windE(i-1) vD(i-VARIO_WINDOW)];
    
    vtot(i) = sqrt(sum(vNow.^2));
    dKE = 0.5 * (sum(vNow.^2) - sum(vOld.^2));
    dKEWind = 0.5 * (sum(vNowWind.^2) - sum(vOldWind.^2));
    dPE = 9.81 * (gpsAlt(i) - gpsAlt(i - VARIO_WINDOW));
    
    if(windE(i-1) > 1)
        sup = 1;
    end

    power = (dKE + dPE) / (elapsed(i) - elapsed(i - VARIO_WINDOW));
    varioComp(i) = power / 9.81;
    powerWind = (dKEWind + dPE) / (elapsed(i) - elapsed(i - VARIO_WINDOW));
    varioCompWind(i) = powerWind / 9.81;
    
    theta1 = atan2(N(i) - N(i - WIN1), E(i) - E(i - WIN1));
    theta2 = atan2(N(i - WIN1) - N(i - WIN2), E(i - WIN1) - E(i - WIN2));
    dTheta = abs(angdiff(theta1, theta2)) * 180 / pi;
    heading(i) = atan2(N(i) - N(i - HEADING_WIN), E(i) - E(i - HEADING_WIN)) * 180 / pi;
    
    curE = windE(i - 1);
    curN = windN(i - 1);
    
    if (dTheta > 45) && (vtot(i) > 15)
        isCircling(i) = 1;
    end
    
    if (isCircling(i) == 1)
        for wb = 1:length(WIND_BINS)
            curHeading = wrapTo180(heading(i) - WIND_BINS(wb));
            oldHeading = wrapTo180(heading(i - 1) - WIND_BINS(wb));
            signNew = sign(curHeading);
            signOld = sign(oldHeading);
            
            if (signNew ~= signOld) && (abs(curHeading) < 90)
                oldIdx = binIdx(wb);
                dt = elapsed(i) - elapsed(oldIdx);
                
                if (dt < 40) && (dt > 7) %previous circle happened in last minute
                    dN = N(i) - N(oldIdx);
                    dE = E(i) - E(oldIdx);
                    dt = elapsed(i) - elapsed(oldIdx);

                    vNwind = dN / dt;
                    vEwind = dE / dt;
                    
                    if sqrt(vNwind^2 + vEwind^2) < 5
                        curE = curE + (vEwind - curE) / WIND_DAMP;
                        curN = curN + (vNwind - curN) / WIND_DAMP;
                    end
                    
                    scatter3(E(i),N(i), -D(i),'r*')
                    
                    if 0
                       clf;
                       plot(E(i-500:i), N(i-500:i)); hold on;
                       
                       plot(E(i),N(i),'r*')
                       plot(E(oldIdx),N(oldIdx),'g*')
                       
                       fprintf('Time %.1f dt %.1f vN %.1f vE %.1f \n', elapsed(i), dt, vN, vE);
                       sup = 1;
                    end
                end

                binIdx(wb) = i; 
            end
        end
    else
        binIdx = binIdx * 0 + 1; % If not circling, restart wind ident
    end
    
    windN(i) = curN;
    windE(i) = curE;
end

windMag = sqrt(windN.^2 + windE.^2);
windDir = atan2d(windN, windE);



TECclip = max(min(varioComp, 3), -0);
figure(1);
scatter3(E, N, -D, 3, TECclip);
xlabel('East (m)'); ylabel('North (m)'); zlabel('Up (m)');
axis equal; colorbar; title('Position vs Vario in m/s');

figure;
ax1 = subplot(2, 1, 1);
plot(elapsed, varioComp); hold on;grid on;
plot(elapsed, varioCompWind);
legend('No wind comp', 'wind comp');
ylabel('Vario in m/s')
ylim([-3 3])

ax2 = subplot(2, 1, 2);
plot(elapsed, vtot); grid on;
xlabel('Time in s')
ylabel('3D V in m/s')
ylim([20 38])
linkaxes([ax1 ax2], 'x')

figure;
scatter3(E, N, 1:length(N), 3, isCircling);
title('Position vs isCircling');

figure;
plot(windN); hold on; grid on;
plot(windE);
plot(isCircling);