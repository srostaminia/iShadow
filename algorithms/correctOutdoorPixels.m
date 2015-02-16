

close all;
clear;


rootDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/';


dirList={fullfile(rootDir,'data'), fullfile(rootDir,'labelPupil')};

%function run_ann_sweep(result_dir, X, gout,avgRadEllipse,nDim,scaleVect)
addpath('~/iShadow/algorithms/cider');
addpath('../ann/lib');
addpath('../ann/run_ann');


%%
% subName='addison';
% 
% subFolderName=strcat(subName,'_','outdoors','_raw.mat');
% out_data=load(fullfile(dataDir,subFolderName));
% 
% 
% X=out_data.X;
% %X([17 22:26 64 203:204 239 287 487 552 561],:)=[];
% 
% %correct outdoor pixels
% brokenPixels=(X(:,111*[1:4]+48 ));
% X(:,111*[1:4]+48 )=repmat(mean(X,2),[1 4]);
% X(:,111*[0:111]+96 )=X(:,111*[0:111]+97 );
%%
figureOn=1;

for m=1:length(dirList)
    
    dir=dirList{m};
    fprintf('DIR changed\n%s\n',dir);
    cd ~
    cd(dir);
    
    
    outdoors_files = cellstr(strsplit(ls('*outdoor*.mat')));
    %outdoors_files
    
    for j=1:length(outdoors_files)-1
        fprintf('%s\n',outdoors_files{j});
        
        data=load(outdoors_files{j});
        %X([17 22:26 64 203:204 239 287 487 552 561],:)=[];
        X=data.X;
        
        if figureOn
            figure;
            imagesc(reshape((X(1,:))',111,112,[]));
            colormap gray
        end
        
       % X(1,111*[1]+48)
        
        
        %correct outdoor 4 broken pixels: replace with mean
        brokenPixels=(X(:,111*[1:4]+48 ));
        X(:,111*[1:4]+48 )=repmat(mean(X,2),[1 4]);
        X(:,111*[0:111]+96 )=X(:,111*[0:111]+97 );
        
       %X(1,111*[1]+48)
        
        if figureOn
            figure;
            imagesc(reshape((X(1,:))',111,112,[]));
            colormap gray
        end
        
        
        save(outdoors_files{j},'X','-append');

        clearvars  X
%         data=load(outdoors_files{j});
%         XX=(data.X);
%         XX(1,111*[1]+48)
%         XX(1,159)
    end
    
end
        