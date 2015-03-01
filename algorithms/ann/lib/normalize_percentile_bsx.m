%function [pixels] = normalize_percentile_bsx(pixels,percent)
 
clear;
close all;


data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/akshaya_calib_pupil.mat');

X=data.X;
X(:,111*[1:111]+96 )=X(:,111*[1:111]+97 );

addpath('/Users/ytun/iShadow/algorithms/ann/lib');

percent=5;

nImages=2;%size(X,1);

nBins=7;%round((maxPixel-minPixel));


pixels=X(1:2,:);


cellPixelRows = mat2cell(pixels, ones(1,nImages), 111*112);


[hisMixed]=cellfun(@countHist,cellPixelRows,repmat({nBins},nImages,1),'UniformOutput',false);

hisMixed=cell2mat(hisMixed);
his=(hisMixed(1:2:end,1:end));
hisEdge=(hisMixed(2:2:end,1:end));

per5 = round(prctile(pixels,percent,2));
per95 = round(prctile(pixels,100-percent,2));
medianV=median(pixels,2);

% 

[~,ind_clipStart,~]=find(hisEdge<=repmat(per5,1,size(hisEdge,2)),2,'last');
[~,ind_clipEnd,~]=find(hisEdge>=repmat(per95,1,size(hisEdge,2)),2,'first');
% 
%make it subtract 5%
ind_clipEnd(ind_clipEnd>=repmat(nBins,size(ind_clipEnd)))=nBins;

ind_clip=zeros(nImages,2);
ind_clip(:,1)=ind_clipStart;
ind_clip(:,2)=ind_clipEnd;

% [7113        1474]
%  2287        2959 4455        1347
%  
cellInd_clip=mat2cell(ind_clip, ones(1,size(ind_clip,2)));

cellHis = mat2cell(his, ones(1,size(his,1)), size(his,2));
clipRawHis=cellfun(@clipHistogram,cellHis,cellInd_clip,'UniformOutput',false);

clipHis=cell2mat(clipRawHis);

%ensure the number of pixels is unchanged after clipping
% totalClipNo=12432-sum(clipRawHis,2);
% 
% clipHis=clipRawHis+floor(totalClipNo./size(clipRawHis,2)); %even out extra pixels across histogram
% rems=rem(totalClipNo,repmat(size(clipHis,2),size(totalClipNo)));
% midCol=ceil(111*112/2);
% rems
% clipHis(:,midCol)=clipHis(:,midCol)+rems;

%normalize to use histeq which is intended for intensity image input
maxPixel=max(pixels,2);
minPixel=min(pixels,2);
newPixels=(pixels-minPixel)./(maxPixel-minPixel);

cellPixelRows = mat2cell(newPixels, ones(1,nImages), 111*112);
cellclipHis = mat2cell(clipHis, ones(1,nImages),size(clipHis,2));

[newPixels]=cellfun(@histeq, cellPixelRows,cellclipHis,'UniformOutput',false);
newPixels=cell2mat(newPixels);
%renormalize to old pixel range and subtract median
maxPixel=max(pixels,2);
minPixel=min(pixels,2);
newPixels=(newPixels-minPixel)./(maxPixel-minPixel);
newPixels=newPixels.*repmat((per95-per5),1,size(newPixels,2))+repmat(per5,1,size(newPixels,2));
newPixels=newPixels-repmat(medianV,1,size(newPixels,2));

%renormalize
maxPixel=max(newPixels,2);
minPixel=min(newPixels,2);
newPixels=(newPixels-minPixel)./(maxPixel-minPixel);

newPixels=newPixels*(2)-1;




