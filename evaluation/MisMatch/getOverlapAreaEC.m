function area=getOverlapAreaEC(w,h,circleSt,ellipseSt)

r_circ=circleSt.radius;
x_circ=circleSt.centX;
y_circ=circleSt.centY;

x_ell=ellipseSt.centX;
y_ell=ellipseSt.centY;
majorlen_ell=ellipseSt.majorlen;
minorlen_ell=ellipseSt.minorlen;
phi_ell=ellipseSt.phi;

area=0;

%matrix for ellipse 
[x,y] = meshgrid(-w/2+0.5:w/2-0.5,-h/2+0.5:h/2-0.5);
el=zeros(h,w);

isinellipse=((x-x_ell)/majorlen_ell).^2+((y-y_ell)/minorlen_ell).^2<=1;
el(isinellipse)=1;
el=imrotate(el,phi_ell,'crop');

%matrix for circle
circ=zeros(h,w);

[x,y] = meshgrid(1:w,1:h);
isincircle=(x-x_circ).^2+(y-y_circ).^2<=r_circ^2;
circ(isincircle)=1;

mismatchBW=xor(el,circ);
overlapBW=and(el,circ);

mismatchArea=sum(mismatchBW(:));
overlapArea=sum(overlapBW(:));

figure;
subplot(2,2,1);
imagesc(el); 
axis equal
axis tight

subplot(2,2,2);
imagesc(circ);
axis equal
axis tight

subplot(2,2,3);
imagesc(mismatchBW); 
axis equal
axis tight
hold on;
pdeellip(x_ell,y_ell,majorlen_ell,minorlen_ell,phi_ell);
hold off;

subplot(2,2,4);
imagesc(overlapBW); 
axis equal
axis tight


fprintf('Overlap Area=%i, Mismatch Area=%i\n',overlapArea,mismatchArea);
%pdeellip(xc,yc,majorlen_ell,minorlen_ell,phi_ell)
%pdecirc(xc,yc,radius)

%colormap(bone) 


end
