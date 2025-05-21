clear all
close all
clc

LinearMotionExample

figure()

origin = [11.785777,44.133148,0];
origin = [11.615665, 43.315353,0];

[latT,lonT] = local2latlon(DATA.X_t,DATA.Y_t,DATA.Y_t.*0,origin);
geoplot(latT,lonT,'Color','red','Linewidth',2)
hold on
geoplot(latT(1),lonT(1),'Color','red','Linewidth',2,'Marker','*')
[lat,lon] = local2latlon(DATA.X_o,DATA.Y_o,DATA.Y_t.*0,origin);
geoplot(lat,lon,'Color','blue','Linewidth',2)
geoplot(lat(1),lon(1),'Color','blue','Linewidth',2,'Marker','+')

UniformlyAcceleratedMotionExample

[latT,lonT] = local2latlon(DATA.X_t,DATA.Y_t,DATA.Y_t.*0,origin);
geoplot(latT,lonT,'Color','magenta','Linewidth',2,'LineStyle',':')


PolynomialMotionExample

[latT,lonT] = local2latlon(DATA.X_t,DATA.Y_t,DATA.Y_t.*0,origin);
geoplot(latT,lonT,'Color','black','Linewidth',2,'LineStyle','--')


[lat,lon] = local2latlon(DATA.X_o,DATA.Y_o,DATA.Y_t.*0,origin);
geoplot(lat,lon,'Color','blue','Linewidth',2)
geoplot(lat(1),lon(1),'Color','blue','Linewidth',2,'Marker','+')



geobasemap topographic
