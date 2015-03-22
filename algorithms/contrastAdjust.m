nSubplot=5;

subName='addison';

addpath('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data');
addpath('/Users/ytun/iShadow/algorithms/ann/lib');

labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data';
addpath(labelPupilDir);

dataCalib=load(fullfile(labelPupilDir,strcat(subName,'_ohsocold_raw.mat')));

%im=imresize(im,[50 50],'bilinear')

% med

% figure;
% imhist(im);
% 
% figure;
% imhist(mat2gray(im));

%im=mean_contrast_adjust_nosave(im);

% figure;
% imhist(mat2gray(im));

% figure;
% imshow(im);


%%
C=-128;
F=(259*(C+255))/(255*(259-C));

im=reshape((dataCalib.X(1,:))',111,112);%rgb2gray(imread('akshaya_calib.png'));
imOri=im;

figure;
imhist(im);

im=imadjust(im);

figure;
imhist(im);

%%
im=F*(im-128)+128;

figure;
subplot(1,nSubplot,1);
imagesc(im);
title('dimmed');

C=180;
F=(259*(C+255))/(255*(259-C));
imAdjusted=F*(im-128)+128;

subplot(1,nSubplot,2);
imshow(imAdjusted);
title('dimmed-> adjusted');

maxIm=max(max(imAdjusted));
imAdjusted=255*imAdjusted/maxIm;

subplot(1,nSubplot,3);
imshow(imAdjusted);
title('adjusted->normalized');


subplot(1,nSubplot,4);
imshow(imadjust(imOri));
title('ori mat2gray');

subplot(1,nSubplot,5);
imshow(imOri);
title('original');
