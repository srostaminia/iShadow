function [newPixels,sizeHist] = normalize_percentile(pixels,percent,sizes,methodd)
%function [newPixels,sizeHist] = normalize_percentile(pixels,percent)
%function [newPixels] = normalize_percentile(pixels)
%percent=5;
% X=bsxfun(@percentile_row,mean(X,2));
% X=bsxfun(@rdivide,X,std(X,0,2));



% function out=normalize_percentile(x,y)
% 
%     out=x-y;%-mean(x,1);
% 
% end

%function newPixels=normalize_percentile(pixels,percent)
%pixels in row shape

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
% pixels=X(1,:);
% 
% Ori=reshape(pixels',111,112);
% 
% s1=3;
% s2=3;
% 
% figure;
% subplot(s1,s2,1);
% imagesc(Ori);
% 
% subplot(s1,s2,2);
% histogram(Ori);
% 
% filt=medfilt2(Ori,[15 15]);
% subplot(s1,s2,3);
% imagesc(filt);
% 
% subplot(s1,s2,4);
% histogram(filt);
% 
% filtEq=(histeq(mat2gray(Ori),2*(imhist(mat2gray(Ori))).^2));
% subplot(s1,s2,5);
% imagesc(filtEq);
% 
% subplot(s1,s2,6);
% histogram(filtEq);
% 
% colormap gray

%%
% pixels=X(100,:);
% 
% Ori=reshape(pixels',111,112);
% 
% figure;
% subplot(s1,s2,1);
% imagesc(Ori);
% 
% subplot(s1,s2,2);
% histogram(Ori);
% 
% filt=medfilt2(Ori,[15 15]);
% subplot(s1,s2,3);
% imagesc(filt);

% subplot(s1,s2,4);
% histogram(filt);



%%
% GIm=imresize(Ori,[sizes(1) sizes(2)],methodd);
% pixels=round(reshape(GIm,1,[]));

% GIm2=imresize(Ori,[sizes(1) sizes(2)],'nearest');
% pixels2=round(reshape(GIm2,1,[]));
% 
% GIm3=imresize(Ori,[sizes(1) sizes(2)],'bilinear');
% pixels3=round(reshape(GIm3,1,[]));

% figure;
% subplot(s1,s2,1)
% imagesc(Ori);
% colormap gray
% title('original')
% 
% subplot(s1,s2,2)
% imagesc(reshape(pixels',50,50));
% colormap gray
% title('bicubic- weighted average of 4-4 neighbors')
% 
% 
% subplot(s1,s2,3)
% imagesc(reshape(pixels2',50,50));
% colormap gray
% title('nearest')
% 
% subplot(s1,s2,4)
% imagesc(reshape(pixels3',50,50));
% colormap gray
% title('bilinear- weighted average of 2-2 neighbors')

%% Plot or not

plotOn=0;

%pixels=X(1,:);



per5 = round(prctile(pixels,percent));
per95 = round(prctile(pixels,100-percent));
medianV = prctile(pixels,50);
maxPixel=max(pixels);
minPixel=min(pixels);
nBins=2*(maxPixel-minPixel);


% size(pixels)
% pixels(1)
% nBins
[his,hisEdge]=histcounts(pixels,nBins);
% hisObj=histogram(pixels,nBins);
% 
% his=hisObj.Values;
% hisEdge=hisObj.BinEdges;

% pixels(pixels<per5)=per5;
% pixels(pixels>per95)=per95;

%hisRange=his.BinLimits
%cumHistVal=sum(his,1)
%per51=cumHistVal*percent/100;

%add the miss number to his

%clip histogram

% per5
% per95
ind_clip1=find(hisEdge==per5);
ind_clip2=find(hisEdge==per95-1);
clipRawHis=his( ind_clip1: ind_clip2);%round(nBins*percent/100)

% imadjust
% clipRawHis=(clipRawHis.^1.3);

% figure;
% subplot(1,2,1)
% bar(his);
% xlim([0 200])
% 
% subplot(1,2,2)
% bar(clipRawHis);
% xlim([0 200])



%clipRawHis=abs(clipRawHis-min(clipRawHis(1),clipRawHis(end)));
%ensure the number of pixels is unchanged after clipping

%length(clipHis)==per95-per5
totalClipNo=sum(his)-sum(clipRawHis);
%sum(clipRawHis)+1280

clipHis=clipRawHis+floor(totalClipNo/length(clipRawHis));

%sum(clipHis)

rems=rem(totalClipNo,length(clipHis));

midInd=ceil(length(clipHis)/2);


%length(clipHis);
clipHis(midInd)=clipHis(midInd)+rems;

%sum(clipHis)==111*112

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

if plotOn
    subplot(2,3,4)
    histogram(newPixels,nb)
end

newPixels=newPixels-medianV;
newPixels=mat2gray(newPixels);
newPixels=newPixels*(2)-1;

if plotOn
    subplot(2,3,5)
    histogram(newPixels,nb);
    xlim([-1 1])  
end

sizeHist=length(clipHis);

end
% Ori=reshape(newPixels',50,50);
% 
% figure;
% imagesc(Ori);
% colormap gray
%end
%%

% 	X=bsxfun(@minus,X,mean(X,2));
% 	X=bsxfun(@rdivide,X,std(X,0,2));

