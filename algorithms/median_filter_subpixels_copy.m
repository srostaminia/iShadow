%function X_adjust=median_filter_subpixels(X,ind)

clear;
close all;

labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/';
addpath(labeldir);

data=load(fullfile(labeldir,'addison_ohsocold_raw.mat'));
X=data.X;
plotOn=1;


%%
%X=[100 20 40 1 30 30; [100 20+10 40-2 1-1 30+5 30+4] ; [100 20 40 1 30 30]-10; [100 20+10 40-2 1-1 30+5 30+4] ]

[h,w]=size(X);
ind= 1:h*w;%[111*4+3,111*4+10,111*4+nToMedian6];%[111, 111*2,111*3, 111*112];
%ind=1:111*112;

[row,col]=ind2sub([h w],ind);

%diagonal median
rowVect=[row+1; row-1 ;row+1; row-1; row; row+1; row-1 ;row; row];
colVect=[col+1 ;col+1 ;col-1 ;col-1; col; col ;col ;col+1 ;col-1];

nToMedian=size(rowVect,1);%nToMedian- 4+1 for hydbrid diagonal

rowMat=repmat(row,nToMedian,1);
colMat=repmat(col,nToMedian,1);

rowVect(rowVect<1 | rowVect>h)=rowMat(rowVect<1 | rowVect>h);
colVect(colVect<1 | colVect>w)=colMat(colVect<1 | colVect>w);

indMat_rowToMedian=sub2ind([h w],rowVect,colVect);

X_adjustMat=zeros(size(X,1),size(X,2),nToMedian);

for i=1:nToMedian %4 neighboring values +itself =nToMedian valuesto take median in total
   X_adjustMat(:,ind,i)=X(:,indMat_rowToMedian(i,:)); %*** you bugger!!! for a long time
end

X_adjust=nanmedian(X_adjustMat,3);

%output only ind pixels
X=X_adjust(:,ind(1:end));


save(fullfile('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data','addison_ohsocoldFiltMedBox_pupil.mat'),'X');
%%

% 
% %
% 
% %%
% 
% if plotOn
%     XCombo=mat2gray(reshape((X)',111,112,[]));
%     X_adjustCombo=mat2gray(reshape((X_adjust)',111,112,[]));
% 
%     figure;
%     subplot(1,2,1);
%     imshow(XCombo(:,:,101));
%     title('original');
% 
%     subplot(1,2,2);
%     imshow(X_adjustCombo(:,:,101));
%     title('diagonal median filter');
% end
% 
% %%
% clear;
% close all;
% 
% labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data';
% addpath(labeldir);
% 
% data=load('addison_ohsocold_raw.mat');
% 
% X=data.X;
% 
% mat=mat2gray(reshape(X(1,:),111,112));
% 
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
% subplot(2,4,5);%nToMedian);
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





