function newPixelsFinal=clip_perc(X,percentile)

%percentile=0.05;
plotOn=0;%default: no plot

nImages=size(X,1);

pixels=X(1:nImages,:);
perStart=prctile(pixels,percentile,2);
perEnd=prctile(pixels,100-percentile,2);
medianPixel=median(pixels,2);

newPixels=pixels;


repStartPerMat=repmat(perStart,1,size(newPixels,2));
repEndPerMat=repmat(perEnd,1,size(newPixels,2));

%replace all pixel values that are less than 5 percentile with 5 percentile
newPixels(newPixels<repStartPerMat)=repStartPerMat(newPixels<repStartPerMat);
%replace all pixel values that are less than 95 percentile with 95 percentile
newPixels(newPixels>repEndPerMat)=repEndPerMat(newPixels>repEndPerMat);

newPixels=newPixels-repmat(medianPixel,1,size(newPixels,2));

maxPixel=max(newPixels,[],2);
minPixel=min(newPixels,[],2);

%newPixelsFinal=2*(newPixels-repmat(minPixel,1,size(newPixels,2)))./repmat(maxPixel-minPixel,1,size(newPixels,2))-ones(size(newPixels));
newPixelsFinal=(newPixels-repmat(minPixel,1,size(newPixels,2)))./repmat(maxPixel-minPixel,1,size(newPixels,2));


for i=1:3
if plotOn
    
    s1=3;
    s2=3;
    
    picori=reshape(pixels(i,:)',111,112);
    pic=reshape(newPixels(i,:)',111,112);

    figure;
    subplot(s1,s2,1);
    histogram(pixels(i,:));

    subplot(s1,s2,2);
    histogram(newPixels(i,:));

    subplot(s1,s2,3);
    imagesc(picori);
    colormap gray

    subplot(s1,s2,4);
    imagesc(pic);
    colormap gray
    %renormalize to old pixel range
    %pixels=pixels*(perEnd-perStart)+perStart;


    figure;
    histogram(newPixelsFinal(i,:));
end
end



