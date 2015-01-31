function [mismatchArea,overlapArea,trueArea]=getMismatchAreaEC(w,h,circleSt,ellipseSt,plotMode)

r_circ=circleSt.radius;
x_circ=circleSt.centX;
y_circ=circleSt.centY;

x_ell=ellipseSt.centX;
y_ell=ellipseSt.centY;
majorlen_ell=ellipseSt.majorlen;
minorlen_ell=ellipseSt.minorlen;
phi_ell=ellipseSt.phi;

mismatchArea=0;

%matrix for ellipse 
%subtract 0.5 - for zero that is between negative and postive
%axes
[x,y] = meshgrid(-w/2+0.5:w/2-0.5,-h/2+0.5:h/2-0.5);

el=zeros(h,w);
%isinellipse=((x-x_ell)/majorlen_ell).^2+((y-y_ell)/minorlen_ell).^2<=1;
isinellipse=((x-0)/(majorlen_ell*0.5)).^2+((y-0)/(minorlen_ell*0.5)).^2<=1;
el(isinellipse)=1;
el=imrotate(el,phi_ell,'crop');  %***important to rotate before circshift
el=circshift(el,round([h/2+y_ell w/2+x_ell]));

trueArea=sum(el(:));
%trueArea=sum(el(:));
%matrix for circle
circ=zeros(h,w);

[x,y] = meshgrid(1:w,1:h);
isincircle=(x-x_circ).^2+(y-y_circ).^2<=r_circ^2;
circ(isincircle)=1;

mismatchBW=xor(el,circ);
overlapBW=and(el,circ);

%fprintf('ellipseArea%.2f, mismatch%i\n',sum(el(:)),sum(mismatchBW(:)));

mismatchArea=sum(mismatchBW(:));
overlapArea=sum(overlapBW(:));

if strcmp(plotMode,'all')
    figure;
    subplot(2,2,1);
    imagesc(el);
    axis equal
    axis tight
    hold on;
    scatter(x_ell,y_ell,'xr');
    %ellipse(majorlen_ell,minorlen_ell,phi_ell*pi/180, x_ell,y_ell,'y')
    %pdeellip(x_ell,y_ell,majorlen_ell,minorlen_ell,phi_ell);
    hold off;
    subplot(2,2,2);
    imagesc(circ);
    axis equal
    axis tight
    
    subplot(2,2,3);
    imagesc(mismatchBW);
    axis equal
    axis tight
    hold on;
    scatter(x_ell,y_ell,'xr');
    ellipse(majorlen_ell/2,minorlen_ell/2,-1*phi_ell*pi/180, x_ell,y_ell,'g');
    %pdeellip(x_ell,y_ell,majorlen_ell,minorlen_ell,phi_ell);
    hold off;
    
    subplot(2,2,4);
    imagesc(overlapBW);
    axis equal
    axis tight
    hold on;
    scatter(x_ell,y_ell,'xr');
    ellipse(majorlen_ell/2,minorlen_ell/2,-1*phi_ell*pi/180, x_ell,y_ell,'g');
    %pdeellip(x_ell,y_ell,majorlen_ell,minorlen_ell,phi_ell);
    hold off;
    
end

%pdeellip(xc,yc,majorlen_ell,minorlen_ell,phi_ell)
%pdecirc(xc,yc,radius)

%colormap(bone) 


end
