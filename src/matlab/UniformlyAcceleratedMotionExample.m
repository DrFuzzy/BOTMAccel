

%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Global DATA
%%%%%%%%%%%%%%%%%%%%%%%%%%%%
DATA=[];
DATA.T=2;%sampling time in seconds
DATA.time_horizon=3600;%number of seconds
DATA.sigma=0.01;%*deg2rad(.5);%standard deviation for the noise
DATA.time=[0:DATA.time_horizon/DATA.T].*DATA.T;


%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Target DATA
%%%%%%%%%%%%%%%%%%%%%%%%%%%%
xt0=30000;%initial position along x axis
yt0=30000;%initial position along y axis
xdott0=8;%initial velocity along x axis
ydott0=7;%initial velocity along y axis
xddott0=-1E-3;%constant acceleration along x axis
yddott0=1E-3;%constant acceleration along y axis
DATA.psi=[xt0,yt0,xdott0,ydott0,xddott0,yddott0];%real parameter vector

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Target Trajectory (without noise)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

DATA.X_t=[];
DATA.Y_t=[];
for k=0:DATA.time_horizon/DATA.T
    DATA.X_t=[DATA.X_t,DATA.psi(1)+DATA.psi(3)*k*DATA.T+...
        DATA.psi(5)*k^2*DATA.T^2/2];
     DATA.Y_t=[DATA.Y_t,DATA.psi(2)+DATA.psi(4)*k*DATA.T+...
        DATA.psi(6)*k^2*DATA.T^2/2];
end


%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Ownship DATA
%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ownship_initial_position_x=0;
ownship_initial_position_y=0;
ownship_velocity_x=5;
ownship_amplitude=5000;
ownship_omega=0.00050;
theta=-pi/10;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Ownship Trajectory 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ownship_rotation=[cos(theta) -sin(theta); sin(theta) cos(theta)];
DATA.X_o=[];
DATA.Y_o=[];
for k=0:DATA.time_horizon/DATA.T 
    xo=ownship_initial_position_x+ownship_velocity_x*k*DATA.T;
    yo=ownship_amplitude*sin(ownship_omega*xo);
    DATA.X_o=[DATA.X_o,xo];
    DATA.Y_o=[DATA.Y_o,yo];
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Measurements : h ideal, w noise, z noisy
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

DATA.MEASURE=[];
DATA.MEASURE.h=atan((DATA.Y_t-DATA.Y_o)./(DATA.X_t-DATA.X_o));
DATA.MEASURE.w=normrnd(0,DATA.sigma,1,length(DATA.MEASURE.h));
DATA.MEASURE.z=DATA.MEASURE.h+DATA.MEASURE.w;



%data to be printed
PRINT=[DATA.time',DATA.X_o',DATA.Y_o',DATA.MEASURE.z'];

output_file = 'outputUniformlyAccelerated.csv';
% Write the matrix to a CSV file
csvwrite(output_file, PRINT);
% Display a confirmation message
disp(['CSV file created: ', output_file]);
