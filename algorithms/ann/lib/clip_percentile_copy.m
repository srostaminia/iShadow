
% clear;
% close all;
% 
% percent=2;
% 
% data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/addison_benchmark_pupil.mat');
% 
% X=data.X;
% X(:,111*[1:111]+96 )=X(:,111*[1:111]+97 );
% 
% 
% plotOn=1;%default: no plot
% 
% nImages=3;%size(X,1);
% 
% pixels=X(1:nImages,:);
% per5=prctile(pixels,percent,2);
% per95=prctile(pixels,100-percent,2);
% medianPixel=median(pixels,2);
% 
% newPixels=pixels;
% 
% 
% rep5PerMat=repmat(per5,1,size(newPixels,2));
% rep95PerMat=repmat(per95,1,size(newPixels,2));
% 
% %replace all pixel values that are less than 5 percentile with 5 percentile
% newPixels(newPixels<rep5PerMat)=rep5PerMat(newPixels<rep5PerMat);
% %replace all pixel values that are less than 95 percentile with 95 percentile
% newPixels(newPixels>rep95PerMat)=rep95PerMat(newPixels>rep95PerMat);
% 
% newPixels=newPixels-repmat(medianPixel,1,size(newPixels,2));
% 
% maxPixel=max(newPixels,[],2);
% minPixel=min(newPixels,[],2);
% 
% newPixelsFinal=2*((newPixels-repmat(minPixel,1,size(newPixels,2)))./repmat(maxPixel-minPixel,1,size(newPixels,2)))-ones(size(newPixels));
% %newPixelsFinal=newPixels;
% 
% for i=1:3
% if plotOn
%     
%     s1=2;
%     s2=2;
%     
%     picori=reshape(pixels(i,:)',111,112);
%     pic=reshape(newPixels(i,:)',111,112);
% 
%     figure;
%     subplot(s1,s2,1);
%     histogram(pixels(i,:));
% 
%     subplot(s1,s2,2);
%     histogram(newPixelsFinal(i,:));%histogram(newPixels(i,:));
% 
%     subplot(s1,s2,3);
%     imagesc(picori);
%     colormap gray
% 
%     subplot(s1,s2,4);
%     imagesc(pic);
%     colormap gray
%     suptitle(strcat('percent=',num2str(percent)))
%     %renormalize to old pixel range
%     %pixels=pixels*(per95-per5)+per5;
% 
% 
%     %figure;
%     %histogram(newPixelsFinal(i,:));
%     
% end
% end

%%
function newPixelsFinal=clip_percentile(X,percent)

plotOn=0;%default: no plot

nImages=size(X,1);

pixels=X(1:nImages,:);
per5=prctile(pixels,percent,2);
per95=prctile(pixels,100-percent,2);
medianPixel=median(pixels,2);

newPixels=pixels;


rep5PerMat=repmat(per5,1,size(newPixels,2));
rep95PerMat=repmat(per95,1,size(newPixels,2));

%replace all pixel values that are less than 5 percentile with 5 percentile
newPixels(newPixels<rep5PerMat)=rep5PerMat(newPixels<rep5PerMat);
%replace all pixel values that are less than 95 percentile with 95 percentile
newPixels(newPixels>rep95PerMat)=rep95PerMat(newPixels>rep95PerMat);

newPixels=newPixels-repmat(medianPixel,1,size(newPixels,2));

maxPixel=max(newPixels,[],2);
minPixel=min(newPixels,[],2);

newPixelsFinal=2*(newPixels-repmat(minPixel,1,size(newPixels,2)))./repmat(maxPixel-minPixel,1,size(newPixels,2))-ones(size(newPixels));


% for i=1:3
% if plotOn
%     
%     s1=3;
%     s2=3;
%     
%     picori=reshape(pixels(i,:)',111,112);
%     pic=reshape(newPixels(i,:)',111,112);
% 
%     figure;
%     subplot(s1,s2,1);
%     histogram(pixels(i,:));
% 
%     subplot(s1,s2,2);
%     histogram(newPixels(i,:));
% 
%     subplot(s1,s2,3);
%     imagesc(picori);
%     colormap gray
% 
%     subplot(s1,s2,4);
%     imagesc(pic);
%     colormap gray
%     %renormalize to old pixel range
%     %pixels=pixels*(per95-per5)+per5;
% 
% 
%     figure;
%     histogram(newPixelsFinal(i,:));
% end
% end

