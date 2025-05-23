clear all 
close all
clc
sigma=0.01;
%caso2
filename1='../../data/ushant_ais/data/traj_327.txt';
filename2='../../data/ushant_ais/data/traj_11330.txt';

[x1,y1,x2,y2,lat1, lon1, lat2, lon2, t,lon1_raw, lat1_raw,lon2_raw, lat2_raw] = loadTraj(filename1,filename2);

MEASURE=[];
MEASURE.h=atan((y1-y2)./(x1-x2));
MEASURE.w=normrnd(0,sigma,1,length(MEASURE.h));
MEASURE.z=MEASURE.h+MEASURE.w;

%data to be printed
PRINT=[t',x2',y2',MEASURE.z'];

output_file = '../../../data/real_world_example.csv';
% Write the matrix to a CSV file
csvwrite(output_file, PRINT);

figure()
geoplot(lat1,lon1,'Color','red','Linewidth',2)
hold on
geoplot(lat2,lon2,'Color','blue','Linewidth',2)
geoplot(lat1(1),lon1(1),'r*','Linewidth',2)
geoplot(lat2(1),lon2(1),'b*','Linewidth',2)

geobasemap topographic

% function [x_uniform,y_uniform,t_uniform,lat_uniform,lon_uniform] = loadTraj(filename)
% % Read AIS trajectory file
% opts = detectImportOptions(filename, 'Delimiter', ';');
% opts = setvartype(opts, {'x','y','vx','vy','t'}, 'double');
% data = readtable(filename, opts);
% 
% % Extract columns
% lon = data.x;      % Longitude in degrees
% lat = data.y;      % Latitude in degrees
% t = data.t;        % Time in seconds
% 
% % Calculate uniform time vector with exactly 1801 samples
% t_min = min(t);  % Minimum time value
% t_max = max(t);  % Maximum time value
% t_uniform = linspace(t_min, t_max, 1801);  % Create 1801 uniformly spaced time samples
% 
% 
% % Interpolate x, y, and t values onto the uniform time grid
% lon_uniform = interp1(t, data.x, t_uniform, 'linear');
% lat_uniform = interp1(t, data.y, t_uniform, 'linear');
% 
% 
% [x_uniform,y_uniform] = latlon2local(lat_uniform,lon_uniform,lon_uniform.*0,[0,0,0]);
% 
% end

function [x1,y1,x2,y2,lat1, lon1, lat2, lon2, t,lon1_raw, lat1_raw,lon2_raw, lat2_raw] = loadTraj(filename1, filename2)
    % Helper to read and parse data
    function [lon, lat, t] = readData(fname)
        opts = detectImportOptions(fname, 'Delimiter', ';');
        opts = setvartype(opts, {'x','y','vx','vy','t'}, 'double');
        data = readtable(fname, opts);
        lon = data.x;
        lat = data.y;
        t   = data.t;
    end

    % Read both files
    [lon1_raw, lat1_raw, t1_raw] = readData(filename1);
    [lon2_raw, lat2_raw, t2_raw] = readData(filename2);
    t1_raw=t1_raw-t1_raw(1);
    t2_raw=t2_raw-t2_raw(1);
    % Find common overlapping time range
    t_start = max(min(t1_raw), min(t2_raw));
    t_end   = min(max(t1_raw), max(t2_raw));
    if t_end <= t_start
        error('No overlapping time interval found between trajectories.');
    end

    % Create uniform time vector with 1801 samples
    t = linspace(t_start, t_end, 1801);

    % Interpolate lat/lon to the common time vector
    lon1 = interp1(t1_raw, lon1_raw, t, 'linear');
    lat1 = interp1(t1_raw, lat1_raw, t, 'linear');
    lon2 = interp1(t2_raw, lon2_raw, t, 'linear');
    lat2 = interp1(t2_raw, lat2_raw, t, 'linear');

    [x1,y1] = latlon2local(lat1,lon1,lon1.*0,[0,0,0]);
    [x2,y2] = latlon2local(lat2,lon2,lon2.*0,[0,0,0]);

end
