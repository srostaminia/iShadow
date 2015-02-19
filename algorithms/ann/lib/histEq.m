%function newMat=histEq(mat,histToEq)

%http://angeljohnsy.blogspot.com/2011/04/matlab-code-histogram-equalization.html

%GIm=imread('tire.tif');


clear;
close all;

percent=5;

data=load('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/addison_column_pupil.mat');

X=data.X;


GIm=reshape(X(1,:)',111,112);
GIm=round(mat2gray(GIm)*255);

%%

numofpixels=size(GIm,1)*size(GIm,2);


figure,imagesc(GIm);colormap gray;

title('Original Image');


HIm=uint8(zeros(size(GIm,1),size(GIm,2)));

freq=zeros(256,1);

probf=zeros(256,1);

probc=zeros(256,1);

cum=zeros(256,1);

output=zeros(256,1);


%freq counts the occurrence of each pixel value.

%The probability of each occurrence is calculated by probf.


for i=1:size(GIm,1)

    for j=1:size(GIm,2)

        value=GIm(i,j);

        freq(value+1)=freq(value+1)+1;

        probf(value+1)=freq(value+1)/numofpixels;

    end

end


sum=0;

no_bins=255;


%The cumulative distribution probability is calculated. 

for i=1:size(probf)

   sum=sum+freq(i);

   cum(i)=sum;

   probc(i)=cum(i)/numofpixels;

   output(i)=round(probc(i)*no_bins);

end

for i=1:size(GIm,1)

    for j=1:size(GIm,2)

            HIm(i,j)=output(GIm(i,j)+1);

    end

end

figure,imshow(HIm);

title('Histogram equalization');