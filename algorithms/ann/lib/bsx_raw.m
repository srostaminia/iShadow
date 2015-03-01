his=      [1 2 15 30 4 1];
his=[his;his+10];
hisEdge=[10 20 30 40 50 60 70];
hisEdge=[hisEdge;10+hisEdge];

%nPixels=111*112;
nToClip=10;%round(nPixels*0.05);

nImages=2;
% 2,6
%5 ,7
nBins=6;

hisInd=1;
nClipped=0;

hisOri=his;

cumsum_95per=fliplr(cumsum(fliplr(his),2));
cumsum_5per=cumsum(his,2);


matToClip=repmat(nToClip,size(cumsum_5per));

check_5per=cumsum_5per-matToClip;
check_95per=cumsum_95per-matToClip

clippedHis=his;


check_5per
hisOri


%% 5 percent
%if negative->clip them completely

check_5per
check_5per<0

binsAlreadyRemoved=hisOri .*(check_5per<0)
=sum(binsAlreadyRemoved,2);

nLeftToClip=sum(hisOri(check_5per<0),2)+repmat(nToClip,nImages,1)

%clippedHis(check_5per<matToClip)=check_5per(check_5per<matToClip);
clippedHis(clippedHis<0)=0
%check how many bins are removed by summing bins of negative values in
%check how many bins left to clip


%already-clipped bins will be negative
check_clipped=his-repmat(nLeftToClip,1,size(clippedHis,2))
%replace the partially-left-to-clip bin in his
[row,col,~]=find(check_clipped<0,1,'first');
col=col+ones(size(col,2),1);
indexRemClipped=sub2ind(size(clippedHis),row,col);
clippedHis(indexRemClipped)=check_clipped(indexRemClipped)
%% 5 percent
% %if negative->clip them completely
% 
% clippedHis(check_5per<matToClip)=check_5per(check_5per<matToClip);
% 
% clippedHis(clippedHis<0)=0;
% %check how many bins are removed by summing bins of negative values in
% %check how many bins left to clip
% nLeftToClip=sum(hisOri(check_5per<0),2)+repmat(nToClip,nImages,1)
% 
% 
% %already-clipped bins will be negative
% check_clipped=his-repmat(nLeftToClip,1,size(clippedHis,2))
% %replace the partially-left-to-clip bin in his
% [row,col,~]=find(check_clipped<0,1,'first');
% col=col+ones(size(col,2),1);
% indexRemClipped=sub2ind(size(clippedHis),row,col);
% clippedHis(indexRemClipped)=check_clipped(indexRemClipped)

%% 95 percent

clippedHis(check_95per<matToClip)=check_95per(check_95per<matToClip)
clippedHis(clippedHis<0)=0;
%check how many bins are removed by summing bins of negative values in
%check how many bins left to clip
nLeftToClip=sum(hisOri(check_95per<0),2)+repmat(nToClip,nImages,1);

%already-clipped bins will be negative
check_clipped=clippedHis-repmat(nLeftToClip,1,size(clippedHis,2));
%replace the partially-left-to-clip bin in his
[row,col,~]=find(check_clipped<0,1,'first');
col=col+ones(size(col,2),1);
indexRemClipped=sub2ind(size(clippedHis),row,col);
clippedHis(indexRemClipped)=check_clipped(indexRemClipped);

%%

% %%
% 
% %if partial left, sum bins that are negative and replace
% 
% 
% 
% check_5per(check_5per<matToClip)
% 
% his(check_95per<matToClip)=check_95per(check_95per<matToClip)
% his(his<0)=0;
% 
% check_95per
% hisOri
%his(his<0)=check_95per(check_95per<matToClip)



his
hisOri
figure;
subplot(1,2,1)
bar(his);

subplot(1,2,2)
bar(clippedHis);