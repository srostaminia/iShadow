
clear;
close all;

percent=5;

data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/akshaya_calib_pupil.mat');

X=data.X;
X(:,111*[1:111]+96 )=X(:,111*[1:111]+97 );


%%

addpath('/Users/ytun/iShadow/algorithms/ann/lib');
methodd='nearest';
sizes=[111 112];
percent=5;

nImages=size(X,1);

%%
endInd=1;




for i=1:endInd
    i
    newX=normalize_percentile_raw( X(1,:) );
%     [X_new]=arrayfun(@(x) normalize_percentile_raw(x), X(1,:));
    %[newX,sizeHist]=normalize_percentile(X(i,:),percent,sizes,methodd);
    %[newX]=normalize_percentile(X(i,:),percent,sizes,methodd);
    %newX1=mean_contrast_adjust_nosave(X(i,:));
    
end
toc

%%
 
for i=[100 200 400];
    newX=normalize_percentile_raw( X(i,:) );

    sizes=[111 112];
    methodd='nearest';
    sizeHist=max(X(i,:)-min(X(i,:)));

    sub1=2;
    sub2=2;

    im=reshape(X(i,:)',111,112);
    newIm=reshape(newX',sizes(1),sizes(2));%,111,112);

    %newIm1=reshape(newX1(1,:)',111,112);

    %
    Ori=reshape(X(i,:)',111,112);
    GIm=imresize(Ori,[sizes(1) sizes(2)],methodd);
    pixels=round(reshape(GIm,1,[]));

    % GIm2=imresize(Ori,[sizes(1) sizes(2)],'nearest');
    % pixels2=round(reshape(GIm2,1,[]));
    % 
    % GIm3=imresize(Ori,[sizes(1) sizes(2)],'bilinear');
    % pixels3=round(reshape(GIm3,1,[]));


    figure;
    subplot(sub1,sub2,1)
    imshow(mat2gray(im))
    colormap gray
    title('original')

    % subplot(sub1,sub2,2)
    % imagesc(GIm2)
    % colormap gray
    % title(strcat('downscale-',methodd,' method'))

    subplot(sub1,sub2,2)
    imagesc(newIm)
    colormap gray
    title('histogram adjustment result')

    subplot(sub1,sub2,3)
    histogram(im,sizeHist,'BinWidth',5)
    title('original histogram');
   
    subplot(sub1,sub2,4)
    histogram(newIm,sizeHist)
    title('histogram result')

    min(min(adjIm))
    max(max(adjIm))

    subplot(sub1,sub2,8)
    histogram(adjIm)
    ylim([0 800])

   

    suptitle({strcat('10 BINS: Result: Downscaling the image by half and Histogram clipping'),'Downscaling method: nearest approach'});

end
