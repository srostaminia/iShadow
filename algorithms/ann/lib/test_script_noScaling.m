
clear;
close all;

percent=5;

data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/addison_column_pupil.mat');

X=data.X;
X(:,111*[0:111]+96 )=X(:,111*[0:111]+97 );
%%
methodd='bilinear'
sizes=[50 50]

for i=1%:size(X,1)
    
    [newX,sizeHist]=normalize_percentile(X(i,:),percent,sizes,methodd);
    
    newX1=mean_contrast_adjust_nosave(X);
end



sub1=3;
sub2=3;
im=reshape(X(1,:)',111,112);

newIm=reshape(newX(1,:)',111,112);

newIm1=reshape(newX1(1,:)',111,112);


figure;
subplot(sub1,sub2,1)
imagesc(im)
colormap gray
title('original')

subplot(sub1,sub2,2)
imagesc(newIm)
colormap gray
title('histogram clipping result')

subplot(sub1,sub2,4)
histogram(im)

subplot(sub1,sub2,5)
histogram(newIm,sizeHist)

adjIm=imadjust(mat2gray(im));

subplot(sub1,sub2,3)
imagesc(adjIm)
colormap gray
title('histogram exponent result')

min(min(adjIm))
max(max(adjIm))

subplot(sub1,sub2,6)
histogram(adjIm)

subplot(sub1,sub2,7)
imshow(im2bw(adjIm,0.1))
title('thresholding(threshold=0.1)');

%suptitle({'Result: Downscaling the image by half and Histogram clipping','Downscaling method: bilinear approach'});
suptitle({'Result: Histogram clipping (no Downscaling)'});

% A = [1 2 3;
%      4 5 6;
%      7 8 9]
% mean(A,1)
% mean(A,2)
% 
% 
% %B = [0 1 2]
% 
% func=@(x,y) normalize_percentile(x,y);
% C = bsxfun(func, A,1)

%http://stackoverflow.com/questions/2307249/how-to-apply-a-function-to-every-row-in-matlab
%C = bsxfun(@(x,y) x.^y, A, B)

