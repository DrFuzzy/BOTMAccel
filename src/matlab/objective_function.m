function f = objective_function(theta,ownship_x,ownship_y,measure,sigma,time)
%Trajectory of the target based on the parameter vector theta
x_t=theta(1)+time.*theta(3)+time.^2.*theta(5)/2;
y_t=theta(2)+time.*theta(4)+time.^2.*theta(6)/2;
h=atan((y_t-ownship_y)./(x_t-ownship_x));
f=(1/(2*sigma^2))*sum((measure-h).^2);



