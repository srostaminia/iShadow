function [x,y]=gen_circle(cent_x,cent_y,r)

    %Define circle in polar coordinates (angle and radius)
    theta=linspace(0,2*pi,100); %100 evenly spaced points between 0 and 2pi
    rho=ones(1,100)*r;

    %Convert polar coordinates to Cartesian for plotting
    [x,y] = pol2cart(theta,rho);

    x = x + cent_x;
    y = y + cent_y;

end