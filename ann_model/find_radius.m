tmp = find(radius==6);

for i=1:size(tmp)
    idx = tmp(i);
    x = gout(idx, 1);
    y = gout(idx, 2);
    
    imagesc(reshape(X(idx,:), [111 112]))
    colormap('gray')
    theta=linspace(0,2*pi,100); %100 evenly spaced points between 0 and 2pi
    rho=ones(1,100)*radius(idx);
    [circX,circY] = pol2cart(theta,rho);
    circX = circX + x;
    circY = circY + y;
    hold on;
    plot(x, y, 'r+')
    
    plot(circX, circY, 'b-');
    input('meh.','s');
end