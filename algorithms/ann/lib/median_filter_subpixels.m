function X_adjust=median_filter_subpixels(X,ind)

% clear;
% close all;

% % labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/addison_calib_raw.mat';
% % addpath(labeldir);
% 
% data=load('addison_calib_raw.mat');
% 
% X=data.X(1:10,:);
plotOn=0;
%%
%ind= 1:111*112;%[111*4+3,111*4+10,111*4+nToMedian6];%[111, 111*2,111*3, 111*112];

[row,col]=ind2sub([111 112],ind);

%diagonal median
rowVect=[row+1; row-1 ;row+1; row-1; row];
colVect=[col+1 ;col+1 ;col-1 ;col-1; col];

nToMedian=size(rowVect,1);%nToMedian

rowMat=repmat(row,nToMedian,1);
colMat=repmat(col,nToMedian,1);

rowVect(rowVect<1 | rowVect>111)=rowMat(rowVect<1 | rowVect>111);
colVect(colVect<1 | colVect>112)=colMat(colVect<1 | colVect>112);

indMat_rowToMedian=sub2ind([111 112],rowVect,colVect);

X_adjustMat=zeros(size(X,1),size(X,2),nToMedian);

for i=1:nToMedian %4 neighboring values +itself =nToMedian valuesto take median in total
   X_adjustMat(:,ind,i)=X(:,indMat_rowToMedian(i,:)); %*** you bugger!!! for a long time
end

X_adjust=nanmedian(X_adjustMat,3);

%output only ind pixels
X_adjust=X_adjust(:,ind(1:end-1));
%

if plotOn
    XCombo=mat2gray(reshape((X)',111,112,[]));
    X_adjustCombo=mat2gray(reshape((X_adjust)',111,112,[]));

    figure;
    subplot(1,2,1);
    imshow(XCombo(:,:,1));
    title('original');

    subplot(1,2,2);
    imshow(X_adjustCombo(:,:,1));
    title('diagonal median filter');
end

% clear;
% close all;
% 
% % labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/addison_calib_raw.mat';
% % addpath(labeldir);
% 
% data=load('addison_calib_raw.mat');
% 
% X=data.X(1:10,:);

% figure;
% subplot(2,4,1);
% imshow(mat);
% title('median filtered: original');
% 
% subplot(2,4,2);
% imshow(medfilt2(mat, [3 3]));
% title('median filtered: box 3x3');
% 
% subplot(2,4,3);
% imshow(medfilt2(mat, [1 3]));
% title('median filtered: horizontally');
% 
% subplot(2,4,4);
% imshow(medfilt2(mat, [3 1]));
% title('median filtered: vertically');
% 
% direction='cross';
% subplot(2,4,nToMedian);
% imshow(hmf(mat,[3 3],direction));
% title('median filtered: cross');
% 
% direction='diagonal';
% subplot(2,4,6);
% imshow(hmf(mat,[3 3],direction));
% title('median filtered: diagonal');
% 
% direction='mixed';
% subplot(2,4,7);
% imshow(hmf(mat,[3 3],direction));
% title('median filtered: cross-diagonal');
% %%





