function [newPixels] = normalize_percentile_moreBins(pixels,percent,sizes,methodd)
    %pixels=X(1,:);
    plotOn=0;

    per5 = round(prctile(pixels,percent));
    per95 = round(prctile(pixels,100-percent));
    medianV = prctile(pixels,50);
    maxPixel=max(pixels);
    minPixel=min(pixels);
    nBins=7;%round((maxPixel-minPixel));

    [his,hisEdge]=histcounts(pixels,nBins);
     %add the miss number to his

    %clip histogram
    figure;
    bar(his);
    
    his(end)
    max(hisEdge)
    per95
    
    ind_clip1=max(find(hisEdge<=per5))
    ind_clip2=min(find(hisEdge>per95));
    
    if (ind_clip2>nBins)
        ind_clip2=5; 
    end
        
    clipRawHis=his( ind_clip1: ind_clip2);
    
    %ensure the number of pixels is unchanged after clipping

    totalClipNo=12432-sum(clipRawHis);
    clipHis=clipRawHis+floor(totalClipNo/length(clipRawHis)); %even out extra pixels across histogram
    rems=rem(totalClipNo,length(clipHis));

    clipMax=max(clipHis);
    maxInd=(clipHis==clipMax);
    %add to the midInd
    clipHis(maxInd)=clipHis(maxInd)+rems;
    
    %normalize to use histeq which is intended for intensity image input
    newPixels=mat2gray(pixels);
    newPixels=histeq(newPixels,clipHis);

    %renormalize to old pixel range
    newPixels=mat2gray(newPixels);
    newPixels=newPixels*(per95-per5)+per5;

%     if plotOn
%         figure;
%         subplot(2,3,1)
%         histogram(pixels,nBins);
%         hold on
%         plot(per5,0,'x')
%         plot(per95,0,'og')
%         hold off
% 
%         subplot(2,3,2)
%         bar(clipHis)
% 
%         subplot(2,3,3)
%         histogram(newPixels)
%         hold on
%         plot(per5,0,'x')
%         plot(per95,0,'og')
%         hold off
%     end

    nb=max(newPixels)-min(newPixels);

%     if plotOn
%         subplot(2,3,4)
%         histogram(newPixels,nb)
%     end

    newPixels=newPixels-medianV;
    newPixels=mat2gray(newPixels);
    newPixels=newPixels*(2)-1;

%     if plotOn
%         subplot(2,3,5)
%         histogram(newPixels,nb);
%         xlim([-1 1])  
%     end

    sizeHist=length(clipHis);




