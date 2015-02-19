%function [newPixels,sizeHist] = normalize_percentile(pixels,percent,sizes,methodd)
function [newPixels,sizeHist] = normalize_percentile(pixels,percent,sizes,methodd)

%pixels=X(1,:);
    plotOn=0;

    per5 = round(prctile(pixels,percent));
    per95 = round(prctile(pixels,100-percent));
    medianV = prctile(pixels,50);
    maxPixel=max(pixels);
    minPixel=min(pixels);
    nBins=3;%round((maxPixel-minPixel)/30);

    [his,hisEdge]=histcounts(pixels,nBins);
     %add the miss number to his

     
     
     
     
    %clip histogram

    ind_clip1=find(hisEdge<=per5);
    ind_clip2=find(hisEdge>=per95-1);
    
    
    clipRawHis=his( ind_clip1: ind_clip2);%round(nBins*percent/100)
    %ensure the number of pixels is unchanged after clipping

    %length(clipHis)==per95-per5
    totalClipNo=12432-sum(clipRawHis);
    %sum(clipRawHis)+1280

    clipHis=clipRawHis+floor(totalClipNo/length(clipRawHis));

    %sum(clipHis)
     
    %%
    rems=rem(totalClipNo,length(clipHis));

    midInd=ceil(length(clipHis)/2);
    %length(clipHis);
    clipHis(midInd)=clipHis(midInd)+rems;

    %normalize to use histeq which is intended for intensity image input
    newPixels=mat2gray(pixels);


    newPixels=histeq(newPixels,clipHis);


    %renormalize 
    newPixels=mat2gray(newPixels);
    %renormalize to old pixel range
    newPixels=newPixels*(per95-per5)+per5;

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


