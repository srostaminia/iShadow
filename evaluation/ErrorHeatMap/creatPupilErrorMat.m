%This code produces heat map of gaze error of the eyeglasses
%wrt outside world coordinates.
%Yamin Tun

%useful commands: whos -file 'filename'

function [errMat,dist]=createPupilErrorMat(filename,dataSel)

load(filename);

if strcmp(dataSel{1},'train')
    if strcmp(dataSel{2},'yhat')
        pos=these_results.yhat_train;
    elseif strcmp(dataSel{2},'y')
        pos=these_results.ytrain;
    end
    dist=these_results.dist_train;
    
elseif strcmp(dataSel{1},'test')
    if strcmp(dataSel{2},'yhat')
        pos=these_results.yhat_test;
    elseif strcmp(dataSel{2},'y')
        pos=these_results.ytest;
    end
    dist=these_results.dist_test;
end

% size(these_results.yhat_train)
% size(these_results.yhat_test)

%resolution
%res=200;
%Known maximum coordinate value
%maxPos=112;
%matsize=round(res*maxPos);
matsize=112;%round(res*max(max(pos)));
res=round(112/max(max(pos)));

max(max(pos))
max(pos(:))
%create the matrix with size of matsizexmatsize and fill all with -0.1
errMat=repmat(-0.1,matsize,matsize); %not to scale down too much->-1

%Sum of all errors
distsum=zeros(matsize,matsize);
%Number of error values that exist for each coordinate
N=zeros(matsize,matsize);

%run loop downward along a column
for n=1:size(dist,1)
    i=round(res*pos(n,1));
    j=round(res*pos(n,2));
    
    %if there is more than one value for one coordinate, average errors
    
    %adding the errors that correspond to the same coordinates
    distsum(i,j)=distsum(i,j)+dist(n,1);     
    %counting how many coordinates overlap in the data set
    N(i,j)=N(i,j)+1;
    
    %mean(distuffer)
end

%Averaging the error for coordinates that have more than one error values
%Dividing the sum of errors for the overlapped coordinates by the number of
%overlapped coordinates
errMat(N~=0)=distsum(N~=0)./N(N~=0);

fprintf('%f of coordinates are covered by the data.\n', sum(size(find(N~=0),1)) / matsize^2 );



%% Test
% dist=[1; 2; 3; 4; 3];
% pos=[0.1 0.1;0.3 0.9;0.9 0.1;0.8 0.9; 0.1 0.12];

%dist(pos(:,1)==1.0025)
%max(max(pos))
