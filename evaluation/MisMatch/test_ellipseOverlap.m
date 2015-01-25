dir='/Users/ytun/Documents/Dropbox/Vision/MobiSys2015_labeling/label'
filename='malai_dark_raw_ellipse.mat'

data=fullfile(dir,filename);

%data.;

%%


%Test script
clear;
addpath('/Users/ytun/Documents/Dropbox/Vision/iShadow2015/')

w=2000;
h=1000;

circleSt.radius=1.5*w/10;
circleSt.centX=w/2;
circleSt.centY=h/2;

ellipseSt.centX=w/10;%w/2;%10+w/2;
ellipseSt.centY=w/10;%h/2;10+w/2;
ellipseSt.majorlen=2*w/10;
ellipseSt.minorlen=w/10;
ellipseSt.phi=45;

area=getOverlapAreaEC(w,h, circleSt, ellipseSt);
