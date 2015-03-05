function [newPixels,sizeHist] = normalize_percentile(pixels,percent)

% X=bsxfun(@percentile_row,mean(X,2));
% X=bsxfun(@rdivide,X,std(X,0,2));

%end

% function out=normalize_percentile(x,y)
% 
% out=x-mean(x,1);
% 
% end

%function newPixels=normalize_percentile(pixels,percent)
%pixels in row shape


% function []= percentile
%     %http://onlinestatbook.com/2/introduction/percentiles.html
%     R = percent/100 * (N + 1)
% 
%     %if rank is integer
%     if round(R)==R
%         per5=hisVal(R)

% clear
% percent=5;
% 
% data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/addison_column_pupil.mat');
% 
% X=data.X;

%%
%pixels=X(1,:);


%% Plot or not

plotOn=0;

%%

per5 = prctile(pixels,percent);
per95 = prctile(pixels,100-percent);
medianV = prctile(pixels,50);
maxPixel=max(pixels);
minPixel=min(pixels);
nBins=maxPixel-minPixel;

hisObj=histogram(pixels,nBins);
his=hisObj.Values;
hisEdge=hisObj.BinEdges;

% pixels(pixels<per5)=per5;
% pixels(pixels>per95)=per95;

%hisRange=his.BinLimits
%cumHistVal=sum(his,1)
%per51=cumHistVal*percent/100;

%add the miss number to his

%clip histogram
ind_clip1=find(hisEdge==per5);
ind_clip2=find(hisEdge==per95-1);
clipRawHis=his( ind_clip1: ind_clip2);%round(nBins*percent/100)

%ensure the number of pixels is unchanged after clipping

%length(clipHis)==per95-per5
totalClipNo=sum(his)-sum(clipRawHis);
%sum(clipRawHis)+1280

clipHis=clipRawHis+floor(totalClipNo/length(clipRawHis));

%sum(clipHis)

rems=rem(totalClipNo,length(clipHis));

midInd=ceil(length(clipHis)/2);
clipHis(midInd)=clipHis(midInd)+rems;

%sum(clipHis)==111*112

a^x +b^x=100
a+b=100

if plotOn
    figure;
    subplot(1,3,1)
    bar(his)

    subplot(1,3,2)
    bar(clipRawHis)

    subplot(1,3,3)
    bar(clipHis)
end

%normalize to use histeq which is intended for intensity image input
newPixels=mat2gray(pixels);
newPixels=histeq(newPixels,clipHis);

%renormalize 
newPixels=mat2gray(newPixels);
%renormalize to old pixel range
newPixels=newPixels*(per95-per5)+per5;
%normalPixel=(newPixels-minNew)/(maxNew-minNew);

if plotOn
    figure;
    subplot(2,3,1)
    histogram(pixels,nBins);
    hold on
    plot(per5,0,'x')
    plot(per95,0,'og')
    hold off

    subplot(2,3,2)
    bar(clipHis)

    subplot(2,3,3)
    histogram(newPixels)
    hold on
    plot(per5,0,'x')
    plot(per95,0,'og')
    hold off
end

nb=max(newPixels)-min(newPixels);
subplot(2,3,4)
histogram(newPixels,nb)

newPixels=newPixels-medianV;
newPixels=mat2gray(newPixels);
newPixels=newPixels*(2)-1;

if plotOn
    subplot(2,3,5)
    histogram(newPixels,nb)
    xlim([-1 1])  
    
end

sizeHist=length(clipHis)
end
%%

% 	X=bsxfun(@minus,X,mean(X,2));
% 	X=bsxfun(@rdivide,X,std(X,0,2));

