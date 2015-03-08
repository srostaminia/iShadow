% %BOX median filtere function X_adjust=median_filter_subpixels(X,ind)
% % 
clear;
close all;

labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/';
addpath(labeldir);

data=load(fullfile(labeldir,'addison_ohsocold_raw.mat'));

X=X;
X_ori=X;
plotOn=1;



X_adjust=X;
hsize=3;

for i=1:size(X,1)
    im=reshape(X(i,:),111,112);
    im = padarray(im,[(hsize-1)/2 (hsize-1)/2],'replicate');
    im=medfilt2(im,[hsize hsize]);
    im_filt = im((hsize+1)/2:end-(hsize-1)/2,(hsize+1)/2:end-(hsize-1)/2);
    X_adjust(i,:)=reshape(im_filt,1,111*112);
end



%%
X=X_adjust;
save(fullfile('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data','addison_ohsocoldFiltMedBox_pupil.mat'),'X');



%%

labelPupildir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';
data=load(fullfile(labelPupildir,'addison_ohsocoldFiltMedBox_pupil.mat'));
%data=load('addison_ohsocoldFiltMedDia_raw.mat','X');

save(fullfile(labelPupildir,labelPupilFileName),'-v7','X','-append');


%%
%X=[100 20 40 1 30 30; [100 20+10 40-2 1-1 30+5 30+4] ; [100 20 40 1 30 30]-10; [100 20+10 40-2 1-1 30+5 30+4] ]

%BOX median filtere : following commented is not working- debug later
% h=111;
% w=112;
% ind= 1:h*w;%[111*4+3,111*4+10,111*4+nToMedian6];%[111, 111*2,111*3, 111*112];
%ind=1:111*112;
%nFrames=size(X,1);

% [row,col]=ind2sub([h w],ind);
% 
% row=row';
% col=col';
% %diagonal median
% rowVect=[row+1 row-1 row+1 row-1 row row+1 row-1 row row];
% colVect=[col+1 col+1 col-1 col-1 col col col col+1 col-1];
% 
% nToMedian=size(rowVect,2);%nToMedian- 4+1 for hydbrid diagonal
% 
% rowMat=repmat(row,1,nToMedian);
% colMat=repmat(col,1,nToMedian);
% 
% rowVect(rowVect<1 | rowVect>h)=rowMat(rowVect<1 | rowVect>h);
% colVect(colVect<1 | colVect>w)=colMat(colVect<1 | colVect>w);
% 
% indMat_rowToMedian=sub2ind([h w],rowVect,colVect);
% 
% multi=reshape( meshgrid(1:nFrames,1:111*112), [], 1);
% Mat_indMat_rowToMedian=bsxfun(@times,repmat(indMat_rowToMedian,nFrames,1), multi);
% % X_adjustMat=zeros(numel(X),nToMedian);
% % 
% % for i=1:nToMedian %4 neighboring values +itself =nToMedian valuesto take median in total
% %    X_adjustMat(ind,i)=X(indMat_rowToMedian(1,:)); %*** you bugger!!! for a long time
% % end
% 
% %X_adjust=nanmedian(X(indMat_rowToMedian),2);
% 
% X_vect=reshape(X',1,[]);
% X_adjust=nanmedian(X_vect(Mat_indMat_rowToMedian),2);
% %output only ind pixels
%X=reshape(X' , 111*112,[]);%X_adjust(:,ind(1:end));
% 
%cellPixelRows = mat2cell(pixels, ones(1,nImages), 111*112);

% cellPixelMat = mat2cell(X', 111*112,ones(1,nFrames));
% 
% [imfilt_Mat]=cellfun(@medfilt2,cellPixelMat,repmat({[3 3]},1,nFrames),'UniformOutput',false);
% imfilt_Mat=cell2mat(imfilt_Mat,111,112,[]);
% 
% X_adjust=reshape(imfilt_Mat,[],111*112);
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
% X=X;

%%
% a=((reshape(X(1,:),111,112))-mean(X(1,:),2))/std(X(1,:),0,2);
% a_adjust=((reshape(X_adjust(1,:),111,112))-mean(X_adjust(1,:),2))/std(X_adjust(1,:),0,2);
% 
% figure;
% subplot(1,2,1);
% imagesc(a);
% axis off
% colormap gray
% title('original');
% 
% subplot(1,2,2);
% imagesc(a_adjust);
% axis off
% colormap gray
% title('filtered');

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
%%





