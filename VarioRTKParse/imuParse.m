clear;clc;close all;

data = importdata('imuTest.txt');

elapsed = data(:, 1) / 1000;
staticAlt = data(:, 2);
staticTemp = data(:, 3);
diffPress = data(:, 4);
diffTemp = data(:, 5);
lat = data(:, 6);
lon = data(:, 7);
q1 = data(:, 8);
q2 = data(:, 9);
q3 = data(:, 10);
q4 = data(:, 11);
vx = data(:, 12);
vy = data(:, 13);
vz = data(:, 14);
ax = data(:, 15);
ay = data(:, 16);
az = data(:, 17);
staticVario = data(:, 18);

plot(elapsed, vz); hold on; grid on;
plot(elapsed, staticVario);