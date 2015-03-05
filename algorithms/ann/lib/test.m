
clear;
close all;

percent=5;

data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/akshaya_calib_pupil.mat');

X=data.X;
X(:,111*[1:111]+96 )=X(:,111*[1:111]+97 );

%%
X(1,:);

nBins=5;
tic
[his]=bsxfun(@histcounts,pixels,nBins);
[hisEdge]=bsxfun(@normalize_percentile,pixels,repmat(nBins,1,1));


toc

tic
[his]=bsxfun(@normalize_percentile,pixels,repmat(nBins,1,1));
toc

s=1
% figure;
% bar(is)