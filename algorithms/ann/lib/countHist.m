%function [newPixels] = normalize_percentile(pixels,percent,sizes,methodd)
function [hisMixed] = countHist(pixels,nBins)


    
    [his,hisEdge]=histcounts(pixels,nBins);
     %add the miss number to his
    
    his=[his 0];
    hisMixed=[his;hisEdge];
    
   
   



